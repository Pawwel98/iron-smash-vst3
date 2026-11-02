#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "CervinizatorDSP.h"

class CervinizatorAudioProcessor : public juce::AudioProcessor
{
public:
    CervinizatorAudioProcessor();
    ~CervinizatorAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
   #endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 1.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Thread-safe metering & trigger indicators for the GUI
    float getInputLevelDb() const noexcept        { return inputPeakDb.load(std::memory_order_relaxed); }
    float getOutputLevelDb() const noexcept       { return outputPeakDb.load(std::memory_order_relaxed); }
    float getTotalGainReductionDb() const noexcept { return totalGrDb.load(std::memory_order_relaxed); }
    bool  isLimiterClipping() const noexcept      { return limiterActiveFlag.load(std::memory_order_relaxed); }
    bool  isKickTriggered() const noexcept        { return kickTrigFlag.load(std::memory_order_relaxed); }
    bool  isSnareTriggered() const noexcept       { return snareTrigFlag.load(std::memory_order_relaxed); }

private:
    CervinizatorDSP::CervinizatorChain dspChain;

    std::atomic<float>* inputParam   = nullptr;
    std::atomic<float>* enhanceParam = nullptr;
    std::atomic<float>* outputParam  = nullptr;
    std::atomic<float>* bypassParam  = nullptr;
    std::atomic<float>* limitParam   = nullptr;

    std::atomic<float> inputPeakDb  { -60.0f };
    std::atomic<float> outputPeakDb { -60.0f };
    std::atomic<float> totalGrDb    { 0.0f };
    std::atomic<bool>  limiterActiveFlag { false };
    std::atomic<bool>  kickTrigFlag      { false };
    std::atomic<bool>  snareTrigFlag     { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CervinizatorAudioProcessor)
};
