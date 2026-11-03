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

    // Top-left knob: INPUT (-24 dB .. +24 dB, default 0.0 dB at 12 o'clock)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "INPUT", 1 },
        "Input Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Center One-Knob: CERVINIZE (0% .. 100%, default 0.0% so plugin opens at zero processing!)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "ENHANCE", 1 },
        "Cervinize",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Top-right knob: OUTPUT (-24 dB .. +24 dB, default 0.0 dB at 12 o'clock)
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

void CervinizatorAudioProcessor::startInputCalibration() noexcept
{
    calibSamplesCollected = 0;
    calibMaxPeak = 0.0f;
    calibSumSq = 0.0;
    calibProgress.store(0.0f, std::memory_order_relaxed);
    hasPendingCalibDb.store(false, std::memory_order_relaxed);
    calibState.store(static_cast<int>(CalibState::WaitingForAudio), std::memory_order_relaxed);
}

void CervinizatorAudioProcessor::cancelInputCalibration() noexcept
{
    calibState.store(static_cast<int>(CalibState::Idle), std::memory_order_relaxed);
    calibProgress.store(0.0f, std::memory_order_relaxed);
    hasPendingCalibDb.store(false, std::memory_order_relaxed);
}

bool CervinizatorAudioProcessor::consumePendingCalibDb(float& outDb) noexcept
{
    if (hasPendingCalibDb.exchange(false, std::memory_order_relaxed))
    {
        outDb = calibratedDb.load(std::memory_order_relaxed);
        return true;
    }
    return false;
}

void CervinizatorAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
    dspChain.prepare(sampleRate);
    setLatencySamples(dspChain.getLatencySamples());
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

    float* leftChannel  = buffer.getWritePointer(0);
    float* rightChannel = (totalNumInputChannels > 1) ? buffer.getWritePointer(1) : nullptr;

    // ------------------------------------------------------------------------
    // Shift + LMB Auto-Input Calibration Analysis (measures raw pre-gain drums)
    // ------------------------------------------------------------------------
    const int cState = calibState.load(std::memory_order_relaxed);
    if (cState == static_cast<int>(CalibState::WaitingForAudio) ||
        cState == static_cast<int>(CalibState::Analyzing))
    {
        float blockPeak = 0.0f;
        double blockSumSq = 0.0;
        for (int s = 0; s < numSamples; ++s)
        {
            const float L = leftChannel[s];
            const float R = (rightChannel != nullptr) ? rightChannel[s] : L;
            const float pk = std::max(std::fabs(L), std::fabs(R));
            blockPeak = std::max(blockPeak, pk);
            blockSumSq += 0.5 * static_cast<double>(L * L + R * R);
        }

        // Detect drum audio above -46 dBFS (0.005 linear)
        if (blockPeak > 0.005f)
        {
            if (cState == static_cast<int>(CalibState::WaitingForAudio))
            {
                calibSamplesCollected = 0;
                calibMaxPeak = 0.0f;
                calibSumSq = 0.0;
                calibState.store(static_cast<int>(CalibState::Analyzing), std::memory_order_relaxed);
            }

            calibMaxPeak = std::max(calibMaxPeak, blockPeak);
            calibSumSq += blockSumSq;
            calibSamplesCollected += numSamples;

            const int targetSamples = std::max(1, static_cast<int>(2.2 * currentSampleRate));
            const float prog = std::clamp(static_cast<float>(calibSamplesCollected) / static_cast<float>(targetSamples), 0.0f, 1.0f);
            calibProgress.store(prog, std::memory_order_relaxed);

            // Compute optimal input gain so drum peaks hit -4.2 dBFS and active RMS hits -14.0 dBFS
            // (the exact sweet spot for our Kick/Snare trigger thresholds and Bus Compressor)
            const float measuredPeakDb = CervinizatorDSP::gainToDb(std::max(calibMaxPeak, 0.005f));
            const float measuredRms = static_cast<float>(std::sqrt(calibSumSq / std::max(1, calibSamplesCollected)));
            const float measuredRmsDb = CervinizatorDSP::gainToDb(std::max(measuredRms, 0.001f));

            const float neededForPeakDb = -4.2f - measuredPeakDb;
            const float neededForRmsDb  = -14.0f - measuredRmsDb;
            const float optimalDb = std::clamp(0.65f * neededForPeakDb + 0.35f * neededForRmsDb, -24.0f, 24.0f);

            calibratedDb.store(optimalDb, std::memory_order_relaxed);
            hasPendingCalibDb.store(true, std::memory_order_relaxed);

            if (calibSamplesCollected >= targetSamples)
            {
                calibState.store(static_cast<int>(CalibState::Complete), std::memory_order_relaxed);
            }
        }
    }

    const float inDb        = inputParam->load(std::memory_order_relaxed);
    const float cervinize01 = enhanceParam->load(std::memory_order_relaxed) * 0.01f;
    const float outDb       = outputParam->load(std::memory_order_relaxed);
    const bool  bypassed    = bypassParam->load(std::memory_order_relaxed) > 0.5f;
    const bool  limitOn     = limitParam->load(std::memory_order_relaxed) > 0.5f;

    dspChain.setParameters(inDb, cervinize01, outDb, bypassed, limitOn);

    for (int s = 0; s < numSamples; ++s)
    {
        float L = leftChannel[s];
        float R = (rightChannel != nullptr) ? rightChannel[s] : L;

        dspChain.processStereoSample(L, R);

        leftChannel[s] = L;
        if (rightChannel != nullptr)
            rightChannel[s] = R;
    }
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
