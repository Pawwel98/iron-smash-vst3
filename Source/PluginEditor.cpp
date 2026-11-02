#include "PluginProcessor.h"
#include "PluginEditor.h"

RockDrumEnhancerAudioProcessorEditor::RockDrumEnhancerAudioProcessorEditor(RockDrumEnhancerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customLookAndFeel);
    setSize(480, 520);

    // 1. Top-Left Knob: INPUT (-24 .. +24 dB, default 0.0 dB)
    inputSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    inputSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
    inputSlider.setDoubleClickReturnValue(true, 0.0);
    addAndMakeVisible(inputSlider);

    inputLabel.setText("INPUT", juce::dontSendNotification);
    inputLabel.setJustificationType(juce::Justification::centred);
    inputLabel.setFont(juce::FontOptions(12.5f, juce::Font::bold));
    inputLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    addAndMakeVisible(inputLabel);

    // 2. Top-Right Knob: OUTPUT (-24 .. +24 dB, default 0.0 dB)
    outputSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
    outputSlider.setDoubleClickReturnValue(true, 0.0);
    addAndMakeVisible(outputSlider);

    outputLabel.setText("OUTPUT", juce::dontSendNotification);
    outputLabel.setJustificationType(juce::Justification::centred);
    outputLabel.setFont(juce::FontOptions(12.5f, juce::Font::bold));
    outputLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    addAndMakeVisible(outputLabel);

    // 3. Center Main One-Knob: ENHANCE (0% .. 100%)
    enhanceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    enhanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 22);
    enhanceSlider.setDoubleClickReturnValue(true, 50.0);
    addAndMakeVisible(enhanceSlider);

    enhanceLabel.setText("DRUM SMASH", juce::dontSendNotification);
    enhanceLabel.setJustificationType(juce::Justification::centred);
    enhanceLabel.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    enhanceLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff6b1a));
    addAndMakeVisible(enhanceLabel);

    // 4. Bottom-Left Toggle: BYPASS
    bypassButton.setName("BYPASS");
    bypassButton.setButtonText("BYPASS");
    addAndMakeVisible(bypassButton);

    // 5. Bottom-Right Button: LIMIT (0 dB Drum Bus Limiter)
    limitButton.setName("LIMIT");
    limitButton.setButtonText("LIMIT 0dB");
    addAndMakeVisible(limitButton);

    // Bind controls to AudioProcessorValueTreeState
    inputAttachment   = std::make_unique<SliderAttachment>(audioProcessor.apvts, "INPUT",   inputSlider);
    enhanceAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "ENHANCE", enhanceSlider);
    outputAttachment  = std::make_unique<SliderAttachment>(audioProcessor.apvts, "OUTPUT",  outputSlider);
    bypassAttachment  = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "BYPASS",  bypassButton);
    limitAttachment   = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "LIMIT",   limitButton);

    startTimerHz(30);
}

RockDrumEnhancerAudioProcessorEditor::~RockDrumEnhancerAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void RockDrumEnhancerAudioProcessorEditor::timerCallback()
{
    repaint();
}

void RockDrumEnhancerAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Dark brushed metal chassis background
    juce::ColourGradient bgGrad(juce::Colour(0xff141820), 0.0f, 0.0f,
                                juce::Colour(0xff0b0d12), 0.0f, static_cast<float>(getHeight()), false);
    g.setGradientFill(bgGrad);
    g.fillAll();

    // Industrial border & corner rivets
    auto frame = getLocalBounds().toFloat().reduced(8.0f);
    g.setColour(juce::Colour(0xff262d3b));
    g.drawRoundedRectangle(frame, 10.0f, 2.0f);

    const float rivetLocations[4][2] = {
        { 18.0f, 18.0f },
        { static_cast<float>(getWidth()) - 26.0f, 18.0f },
        { 18.0f, static_cast<float>(getHeight()) - 26.0f },
        { static_cast<float>(getWidth()) - 26.0f, static_cast<float>(getHeight()) - 26.0f }
    };
    g.setColour(juce::Colour(0xff334155));
    for (const auto& pt : rivetLocations)
        g.fillEllipse(pt[0], pt[1], 8.0f, 8.0f);

    // Top Center Title Banner
    g.setColour(juce::Colour(0xfff8fafc));
    g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    g.drawText("IRON SMASH", 0, 18, getWidth(), 24, juce::Justification::centred);

    g.setColour(juce::Colour(0xff64748b));
    g.setFont(juce::FontOptions(11.0f, juce::Font::plain));
    g.drawText("ROCK & METAL DRUM BUS ENHANCER", 0, 40, getWidth(), 16, juce::Justification::centred);

    // Subtle reactive glow behind the central One-Knob proportional to GR & Enhance
    const float grDb = audioProcessor.getTotalGainReductionDb();
    const float enhancePct = static_cast<float>(enhanceSlider.getValue()) * 0.01f;
    const float haloAlpha = juce::jlimit(0.05f, 0.45f, enhancePct * 0.25f + (grDb / 18.0f) * 0.25f);

    const float cx = static_cast<float>(getWidth()) * 0.5f;
    const float cy = 265.0f;
    juce::ColourGradient halo(juce::Colour(0xffff5500).withAlpha(haloAlpha), cx, cy,
                              juce::Colours::transparentBlack, cx + 130.0f, cy + 130.0f, true);
    g.setGradientFill(halo);
    g.fillEllipse(cx - 130.0f, cy - 130.0f, 260.0f, 260.0f);

    // DSP Chain Status Readout Pill above bottom buttons
    g.setColour(juce::Colour(0xff161b24));
    g.fillRoundedRectangle(90.0f, 412.0f, 300.0f, 26.0f, 5.0f);
    g.setColour(juce::Colour(0xff2d3748));
    g.drawRoundedRectangle(90.0f, 412.0f, 300.0f, 26.0f, 5.0f, 1.0f);

    juce::String statusText = "GR: -" + juce::String(grDb, 1) + " dB   |   OUT: "
                            + juce::String(audioProcessor.getOutputLevelDb(), 1) + " dBFS";
    if (audioProcessor.isLimiterClipping())
        statusText += "  [LIM]";

    g.setColour(audioProcessor.isLimiterClipping() ? juce::Colour(0xffff6b1a) : juce::Colour(0xff94a3b8));
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText(statusText, 90, 412, 300, 26, juce::Justification::centred);
}

void RockDrumEnhancerAudioProcessorEditor::resized()
{
    // Top-Left: INPUT small knob
    inputLabel.setBounds(28, 56, 96, 18);
    inputSlider.setBounds(28, 74, 96, 96);

    // Top-Right: OUTPUT small knob
    outputLabel.setBounds(getWidth() - 124, 56, 96, 18);
    outputSlider.setBounds(getWidth() - 124, 74, 96, 96);

    // Center: Massive One-Knob (ENHANCE)
    enhanceLabel.setBounds((getWidth() - 220) / 2, 145, 220, 22);
    enhanceSlider.setBounds((getWidth() - 220) / 2, 168, 220, 228);

    // Bottom: BYPASS switch (left) & LIMIT button (right)
    bypassButton.setBounds(48, 454, 155, 38);
    limitButton.setBounds(getWidth() - 203, 454, 155, 38);
}
