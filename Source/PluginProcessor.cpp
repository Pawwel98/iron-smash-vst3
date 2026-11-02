#include "PluginProcessor.h"
#include "PluginEditor.h"

CervinizatorAudioProcessor::CervinizatorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
                     .withInput("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
#else
    :
#endif
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    inputParam   = apvts.getRawParameterValue("INPUT");
    enhanceParam = apvts.getRawParameterValue("ENHANCE");
    outputParam  = apvts.getRawParameterValue("OUTPUT");
    bypassParam  = apvts.getRawParameterValue("BYPASS");
    limitParam   = apvts.getRawParameterValue("LIMIT");
}

juce::AudioProcessorValueTreeState::ParameterLayout CervinizatorAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Top-left knob: INPUT (-24 dB .. +24 dB, default 0.0 dB at center)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "INPUT", 1 },
        "Input Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Center One-Knob: CERVINIZE (0% .. 100%, default 50%)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "ENHANCE", 1 },
        "Cervinize",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Top-right knob: OUTPUT (-24 dB .. +24 dB, default 0.0 dB at center)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "OUTPUT", 1 },
        "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Bottom-left toggle: BYPASS (off by default)
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "BYPASS", 1 },
        "Bypass",
        false));

    // Bottom-right button: LIMIT (0.0 dBFS Drum Limiter, on by default)
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "LIMIT", 1 },
        "0dB Drum Limit",
        true));

    return { params.begin(), params.end() };
}

void CervinizatorAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    dspChain.prepare(sampleRate);
}

void CervinizatorAudioProcessor::releaseResources()
{
    dspChain.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CervinizatorAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& mainIn  = layouts.getMainInputChannelSet();
    const auto& mainOut = layouts.getMainOutputChannelSet();

    if (mainIn != mainOut)
        return false;

    return (mainOut == juce::AudioChannelSet::mono()
         || mainOut == juce::AudioChannelSet::stereo());
}
#endif

void CervinizatorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples             = buffer.getNumSamples();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    const float inDb        = inputParam->load(std::memory_order_relaxed);
    const float cervinize01 = enhanceParam->load(std::memory_order_relaxed) * 0.01f;
    const float outDb       = outputParam->load(std::memory_order_relaxed);
    const bool  bypassed    = bypassParam->load(std::memory_order_relaxed) > 0.5f;
    const bool  limitOn     = limitParam->load(std::memory_order_relaxed) > 0.5f;

    dspChain.setParameters(inDb, cervinize01, outDb, bypassed, limitOn);

    float* leftChannel  = buffer.getWritePointer(0);
    float* rightChannel = (totalNumInputChannels > 1) ? buffer.getWritePointer(1) : nullptr;

    float inPeak  = 0.0f;
    float outPeak = 0.0f;

    for (int s = 0; s < numSamples; ++s)
    {
        float L = leftChannel[s];
        float R = (rightChannel != nullptr) ? rightChannel[s] : L;

        inPeak = std::max(inPeak, std::max(std::fabs(L), std::fabs(R)));

        dspChain.processStereoSample(L, R);

        leftChannel[s] = L;
        if (rightChannel != nullptr)
            rightChannel[s] = R;

        outPeak = std::max(outPeak, std::max(std::fabs(L), std::fabs(R)));
    }

    inputPeakDb.store(CervinizatorDSP::gainToDb(inPeak), std::memory_order_relaxed);
    outputPeakDb.store(CervinizatorDSP::gainToDb(outPeak), std::memory_order_relaxed);
    totalGrDb.store(dspChain.getCompressorGR() + dspChain.getLimiterGR(), std::memory_order_relaxed);
    limiterActiveFlag.store(dspChain.getLimiterGR() > 0.1f, std::memory_order_relaxed);
    kickTrigFlag.store(dspChain.isKickTriggered(), std::memory_order_relaxed);
    snareTrigFlag.store(dspChain.isSnareTriggered(), std::memory_order_relaxed);
}

juce::AudioProcessorEditor* CervinizatorAudioProcessor::createEditor()
{
    return new CervinizatorAudioProcessorEditor(*this);
}

void CervinizatorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void CervinizatorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CervinizatorAudioProcessor();
}
