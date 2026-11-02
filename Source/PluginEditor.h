#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// ============================================================================
// Custom Rock/Metal Industrial LookAndFeel for Knobs and Toggle Buttons
// ============================================================================
class IronSmashLookAndFeel : public juce::LookAndFeel_V4
{
public:
    IronSmashLookAndFeel()
    {
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe2e8f0));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, const float rotaryStartAngle,
                          const float rotaryEndAngle, juce::Slider& slider) override
    {
        const float radius = static_cast<float>(juce::jmin(width / 2, height / 2)) - 10.0f;
        const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
        const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const bool isMainKnob = (width > 150);

        // Background track arc
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY, radius + 4.0f, radius + 4.0f, 0.0f,
                            rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xff1e242d));
        g.strokePath(bgArc, juce::PathStrokeType(isMainKnob ? 6.0f : 3.5f,
                                                 juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

        // Active glowing value arc
        juce::Path valArc;
        const float startAnchor = isMainKnob ? rotaryStartAngle : (rotaryStartAngle + rotaryEndAngle) * 0.5f;
        valArc.addCentredArc(centreX, centreY, radius + 4.0f, radius + 4.0f, 0.0f,
                             startAnchor, angle, true);
        const juce::Colour glowCol = isMainKnob ? juce::Colour(0xffff5500) : juce::Colour(0xfff59e0b);
        g.setColour(glowCol);
        g.strokePath(valArc, juce::PathStrokeType(isMainKnob ? 6.0f : 3.5f,
                                                  juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        // Metallic knob body
        juce::ColourGradient bodyGrad(juce::Colour(0xff384152), centreX - radius * 0.5f, centreY - radius * 0.5f,
                                      juce::Colour(0xff131720), centreX + radius * 0.5f, centreY + radius * 0.5f, true);
        g.setGradientFill(bodyGrad);
        g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

        // Inner bevel ring
        g.setColour(juce::Colour(0xff4b5563).withAlpha(0.6f));
        g.drawEllipse(centreX - radius + 2.0f, centreY - radius + 2.0f,
                      (radius - 2.0f) * 2.0f, (radius - 2.0f) * 2.0f, 1.5f);

        // Pointer notch
        juce::Path pointer;
        const float pointerLen = radius * 0.72f;
        const float pointerThickness = isMainKnob ? 4.5f : 3.0f;
        pointer.addRoundedRectangle(-pointerThickness * 0.5f, -radius + 6.0f,
                                    pointerThickness, pointerLen * 0.45f, 1.5f);
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(isMainKnob ? juce::Colour(0xffff6b1a) : juce::Colour(0xfffbbf24));
        g.fillPath(pointer);

        juce::ignoreUnused(slider);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool /*shouldDrawButtonAsDown*/) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
        const bool isOn = button.getToggleState();
        const bool isBypassBtn = button.getName() == "BYPASS";

        // Active color: red/amber warning for BYPASS, fiery orange-red for LIMIT
        const juce::Colour activeCol = isBypassBtn ? juce::Colour(0xffef4444) : juce::Colour(0xffff5500);

        g.setColour(isOn ? activeCol.withAlpha(0.22f) : juce::Colour(0xff161a22));
        g.fillRoundedRectangle(bounds, 6.0f);

        g.setColour(isOn ? activeCol : (shouldDrawButtonAsHighlighted ? juce::Colour(0xff64748b) : juce::Colour(0xff334155)));
        g.drawRoundedRectangle(bounds, 6.0f, 1.5f);

        // Status LED dot on the left of the button
        const float ledSize = 8.0f;
        const float ledX = bounds.getX() + 12.0f;
        const float ledY = bounds.getCentreY() - ledSize * 0.5f;
        g.setColour(isOn ? activeCol : juce::Colour(0xff334155));
        g.fillEllipse(ledX, ledY, ledSize, ledSize);

        g.setColour(isOn ? juce::Colours::white : juce::Colour(0xff94a3b8));
        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        g.drawText(button.getButtonText(),
                   bounds.withTrimmedLeft(24.0f),
                   juce::Justification::centred, false);
    }
};

// ============================================================================
// Main Plugin Editor Window
// Layout:
// - Top-Left:  INPUT small rotary knob (-24 dB .. +24 dB, default 0.0 dB)
// - Top-Right: OUTPUT small rotary knob (-24 dB .. +24 dB, default 0.0 dB)
// - Center:    ENHANCE massive One-Knob (0% .. 100%)
// - Bottom-Left:  BYPASS toggle switch
// - Bottom-Right: LIMIT (0 dB Drum Bus Limiter) toggle button
// ============================================================================
class RockDrumEnhancerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                             private juce::Timer
{
public:
    explicit RockDrumEnhancerAudioProcessorEditor(RockDrumEnhancerAudioProcessor&);
    ~RockDrumEnhancerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    RockDrumEnhancerAudioProcessor& audioProcessor;
    IronSmashLookAndFeel customLookAndFeel;

    juce::Slider inputSlider;
    juce::Slider enhanceSlider;
    juce::Slider outputSlider;

    juce::Label inputLabel;
    juce::Label enhanceLabel;
    juce::Label outputLabel;

    juce::ToggleButton bypassButton;
    juce::ToggleButton limitButton;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> inputAttachment;
    std::unique_ptr<SliderAttachment> enhanceAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;
    std::unique_ptr<ButtonAttachment> limitAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RockDrumEnhancerAudioProcessorEditor)
};
