#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "CervinizatorDSP.h"

class CervinizatorAudioProcessor : public juce::AudioProcessor
{
public:
    enum class CalibState : int
    {
        Idle = 0,
        WaitingForAudio = 1,
        Analyzing = 2,
        Complete = 3
    };

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

    // Shift + LMB Auto-Input Calibration API
    void startInputCalibration() noexcept;
    void cancelInputCalibration() noexcept;
    CalibState getCalibrationState() const noexcept { return static_cast<CalibState>(calibState.load(std::memory_order_relaxed)); }
    float getCalibrationProgress() const noexcept   { return calibProgress.load(std::memory_order_relaxed); }
    float getCalibratedInputDb() const noexcept     { return calibratedDb.load(std::memory_order_relaxed); }
    bool consumePendingCalibDb(float& outDb) noexcept;

private:
    CervinizatorDSP::CervinizatorChain dspChain;
    double currentSampleRate = 48000.0;

    std::atomic<float>* inputParam   = nullptr;
    std::atomic<float>* enhanceParam = nullptr;
    std::atomic<float>* outputParam  = nullptr;
    std::atomic<float>* bypassParam  = nullptr;
    std::atomic<float>* limitParam   = nullptr;

    // Auto-calibration state
    std::atomic<int>   calibState          { static_cast<int>(CalibState::Idle) };
    std::atomic<float> calibProgress       { 0.0f };
    std::atomic<float> calibratedDb        { 0.0f };
    std::atomic<bool>  hasPendingCalibDb   { false };

    int    calibSamplesCollected = 0;
    float  calibMaxPeak          = 0.0f;
    double calibSumSq            = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CervinizatorAudioProcessor)
};
