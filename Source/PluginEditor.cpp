#include "PluginProcessor.h"
#include "PluginEditor.h"

CervinizatorAudioProcessorEditor::CervinizatorAudioProcessorEditor(CervinizatorAudioProcessor& p)
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
    inputLabel.setFont(juce::Font(12.5f, juce::Font::bold));
    inputLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    addAndMakeVisible(inputLabel);

    // 2. Top-Right Knob: OUTPUT (-24 .. +24 dB, default 0.0 dB)
    outputSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
    outputSlider.setDoubleClickReturnValue(true, 0.0);
    addAndMakeVisible(outputSlider);

    outputLabel.setText("OUTPUT", juce::dontSendNotification);
    outputLabel.setJustificationType(juce::Justification::centred);
    outputLabel.setFont(juce::Font(12.5f, juce::Font::bold));
    outputLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    addAndMakeVisible(outputLabel);

    // 3. Center Main One-Knob: CERVINIZE (0% .. 100%)
    enhanceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    enhanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 22);
    enhanceSlider.setDoubleClickReturnValue(true, 50.0);
    addAndMakeVisible(enhanceSlider);

    enhanceLabel.setText("CERVINIZE", juce::dontSendNotification);
    enhanceLabel.setJustificationType(juce::Justification::centred);
    enhanceLabel.setFont(juce::Font(15.0f, juce::Font::bold));
    enhanceLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff3b6b));
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

CervinizatorAudioProcessorEditor::~CervinizatorAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void CervinizatorAudioProcessorEditor::timerCallback()
{
    repaint();
}

void CervinizatorAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Dark brushed metal chassis background
    juce::ColourGradient bgGrad(juce::Colour(0xff141622), 0.0f, 0.0f,
                                juce::Colour(0xff090b10), 0.0f, static_cast<float>(getHeight()), false);
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
    g.setFont(juce::Font(21.0f, juce::Font::bold));
    g.drawText("CERVINIZATOR", 0, 18, getWidth(), 24, juce::Justification::centred);

    g.setColour(juce::Colour(0xff94a3b8));
    g.setFont(juce::Font(10.5f, juce::Font::plain));
    g.drawText("ZAKK CERVINI STYLE DRUM BUS + KICK/SNARE REINFORCER", 0, 40, getWidth(), 16, juce::Justification::centred);

    // Reactive glow behind the central One-Knob proportional to GR, Sample Trigger, & Cervinize
    const float grDb = audioProcessor.getTotalGainReductionDb();
    const bool kickOn = audioProcessor.isKickTriggered();
    const bool snareOn = audioProcessor.isSnareTriggered();
    const float enhancePct = static_cast<float>(enhanceSlider.getValue()) * 0.01f;
    const float trigBoost = (kickOn || snareOn) ? 0.15f : 0.0f;
    const float haloAlpha = juce::jlimit(0.05f, 0.50f, enhancePct * 0.25f + (grDb / 18.0f) * 0.20f + trigBoost);

    const float cx = static_cast<float>(getWidth()) * 0.5f;
    const float cy = 265.0f;
    juce::ColourGradient halo(juce::Colour(0xffff2a5f).withAlpha(haloAlpha), cx, cy,
                              juce::Colours::transparentBlack, cx + 135.0f, cy + 135.0f, true);
    g.setGradientFill(halo);
    g.fillEllipse(cx - 135.0f, cy - 135.0f, 270.0f, 270.0f);

    // Status & Sample Trigger Readout Pill above bottom buttons
    g.setColour(juce::Colour(0xff141923));
    g.fillRoundedRectangle(48.0f, 410.0f, 384.0f, 28.0f, 6.0f);
    g.setColour(juce::Colour(0xff2d3748));
    g.drawRoundedRectangle(48.0f, 410.0f, 384.0f, 28.0f, 6.0f, 1.0f);

    // Draw KICK and SNARE sample trigger LED badges on the left side of the status bar
    g.setColour(kickOn ? juce::Colour(0xff22c55e) : juce::Colour(0xff334155));
    g.fillEllipse(60.0f, 420.0f, 8.0f, 8.0f);
    g.setColour(kickOn ? juce::Colours::white : juce::Colour(0xff64748b));
    g.setFont(juce::Font(11.5f, juce::Font::bold));
    g.drawText("KICK", 72, 410, 42, 28, juce::Justification::centredLeft);

    g.setColour(snareOn ? juce::Colour(0xfff59e0b) : juce::Colour(0xff334155));
    g.fillEllipse(118.0f, 420.0f, 8.0f, 8.0f);
    g.setColour(snareOn ? juce::Colours::white : juce::Colour(0xff64748b));
    g.drawText("SNARE", 130, 410, 50, 28, juce::Justification::centredLeft);

    // Draw GR & Output peak on the right side of the status bar
    juce::String statusText = "GR: -" + juce::String(grDb, 1) + " dB  |  OUT: "
                            + juce::String(audioProcessor.getOutputLevelDb(), 1) + " dB";
    if (audioProcessor.isLimiterClipping())
        statusText += " [LIM]";

    g.setColour(audioProcessor.isLimiterClipping() ? juce::Colour(0xffff3b6b) : juce::Colour(0xffcbd5e1));
    g.setFont(juce::Font(11.5f, juce::Font::bold));
    g.drawText(statusText, 185, 410, 238, 28, juce::Justification::centredRight);
}

void CervinizatorAudioProcessorEditor::resized()
{
    // Top-Left: INPUT small knob
    inputLabel.setBounds(28, 56, 96, 18);
    inputSlider.setBounds(28, 74, 96, 96);

    // Top-Right: OUTPUT small knob
    outputLabel.setBounds(getWidth() - 124, 56, 96, 18);
    outputSlider.setBounds(getWidth() - 124, 74, 96, 96);

    // Center: Massive One-Knob (CERVINIZE)
    enhanceLabel.setBounds((getWidth() - 220) / 2, 145, 220, 22);
    enhanceSlider.setBounds((getWidth() - 220) / 2, 168, 220, 228);

    // Bottom: BYPASS switch (left) & LIMIT button (right)
    bypassButton.setBounds(48, 454, 155, 38);
    limitButton.setBounds(getWidth() - 203, 454, 155, 38);
}
