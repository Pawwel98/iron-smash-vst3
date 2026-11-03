#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "EmbeddedGraphics.h"

// ============================================================================
// Custom Slider for INPUT Knob supporting Shift + Left Mouse Button Calibration
// ============================================================================
class CalibratingInputSlider : public juce::Slider
{
public:
    std::function<void()> onShiftClick;

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isShiftDown() && e.mods.isLeftButtonDown())
        {
            if (onShiftClick)
                onShiftClick();
            return;
        }
        juce::Slider::mouseDown(e);
    }
};

// ============================================================================
// Custom Image-Based LookAndFeel rendering the user's Photoshop layers & overlays
// ============================================================================
class CervinizatorImageLookAndFeel : public juce::LookAndFeel_V4
{
public:
    juce::Image knobMainImg, knobMainOvImg;
    juce::Image knobInImg,   knobInOvImg;
    juce::Image knobOutImg,  knobOutOvImg;
    juce::Image bypassOffImg, bypassOnImg;
    juce::Image limitOffImg,  limitOnImg;

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, const float rotaryStartAngle,
                          const float rotaryEndAngle, juce::Slider& slider) override
    {
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const juce::String name = slider.getName();

        const juce::Image* bodyImg = &knobMainImg;
        const juce::Image* ovImg   = &knobMainOvImg;

        if (name == "INPUT")
        {
            bodyImg = &knobInImg;
            ovImg   = &knobInOvImg;
        }
        else if (name == "OUTPUT")
        {
            bodyImg = &knobOutImg;
            ovImg   = &knobOutOvImg;
        }

        if (bodyImg->isValid())
        {
            const float imgW = static_cast<float>(bodyImg->getWidth());
            const float imgH = static_cast<float>(bodyImg->getHeight());
            const float scaleX = static_cast<float>(width)  / imgW;
            const float scaleY = static_cast<float>(height) / imgH;

            // Rotate knob around its exact center, then scale and translate into widget bounds
            juce::AffineTransform transform = juce::AffineTransform::rotation(angle, imgW * 0.5f, imgH * 0.5f)
                                              .scaled(scaleX, scaleY)
                                              .translated(static_cast<float>(x), static_cast<float>(y));
            g.drawImageTransformed(*bodyImg, transform, false);
        }

        // Draw static specular highlight overlay on top without rotating
        if (ovImg->isValid())
        {
            g.drawImage(*ovImg, juce::Rectangle<float>(static_cast<float>(x),
                                                       static_cast<float>(y),
                                                       static_cast<float>(width),
                                                       static_cast<float>(height)));
        }
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/) override
    {
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

        const bool isOn = button.getToggleState();
        const bool isBypass = (button.getName() == "BYPASS");

        const juce::Image& img = isBypass ? (isOn ? bypassOnImg : bypassOffImg)
                                          : (isOn ? limitOnImg  : limitOffImg);

        if (img.isValid())
        {
            g.drawImage(img, button.getLocalBounds().toFloat());
        }
    }
};

// ============================================================================
// CERVINIZATOR v5 Main Editor Window
// ============================================================================
class CervinizatorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                         private juce::Timer
{
public:
    explicit CervinizatorAudioProcessorEditor(CervinizatorAudioProcessor&);
    ~CervinizatorAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    static juce::Image loadEmbeddedImage(const char* const* chunks, std::size_t numChunks);

    CervinizatorAudioProcessor& audioProcessor;
    CervinizatorImageLookAndFeel customLookAndFeel;

    juce::Image bg0Image;
    juce::Image bg100Image;

    CalibratingInputSlider inputSlider;
    juce::Slider enhanceSlider;
    juce::Slider outputSlider;

    juce::ToggleButton bypassButton;
    juce::ToggleButton limitButton;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> inputAttachment;
    std::unique_ptr<SliderAttachment> enhanceAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;
    std::unique_ptr<ButtonAttachment> limitAttachment;

    // Hover Tooltip state (~650 ms delay without clicking)
    int hoveredControlId = -1;
    int hoverTicks = 0;
    juce::String activeTooltipText;
    juce::Rectangle<float> activeTooltipTargetBounds;

    // Calibration completion auto-hide timer
    int calibCompleteHoldTicks = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CervinizatorAudioProcessorEditor)
};
