#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::Image CervinizatorAudioProcessorEditor::loadEmbeddedImage(const char* const* chunks, std::size_t numChunks)
{
    auto bytes = CervinizatorGraphics::decodeBase64Chunks(chunks, numChunks);
    return juce::ImageFileFormat::loadFrom(bytes.data(), bytes.size());
}

CervinizatorAudioProcessorEditor::CervinizatorAudioProcessorEditor(CervinizatorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Decode embedded 2x Photoshop layers
    bg0Image   = loadEmbeddedImage(CervinizatorGraphics::kBg0Chunks,   CervinizatorGraphics::kBg0NumChunks);
    bg100Image = loadEmbeddedImage(CervinizatorGraphics::kBg100Chunks, CervinizatorGraphics::kBg100NumChunks);

    customLookAndFeel.knobMainImg   = loadEmbeddedImage(CervinizatorGraphics::kKnobMainChunks,   CervinizatorGraphics::kKnobMainNumChunks);
    customLookAndFeel.knobMainOvImg = loadEmbeddedImage(CervinizatorGraphics::kKnobMainOvChunks, CervinizatorGraphics::kKnobMainOvNumChunks);
    customLookAndFeel.knobInImg     = loadEmbeddedImage(CervinizatorGraphics::kKnobInChunks,     CervinizatorGraphics::kKnobInNumChunks);
    customLookAndFeel.knobInOvImg   = loadEmbeddedImage(CervinizatorGraphics::kKnobInOvChunks,   CervinizatorGraphics::kKnobInOvNumChunks);
    customLookAndFeel.knobOutImg    = loadEmbeddedImage(CervinizatorGraphics::kKnobOutChunks,    CervinizatorGraphics::kKnobOutNumChunks);
    customLookAndFeel.knobOutOvImg  = loadEmbeddedImage(CervinizatorGraphics::kKnobOutOvChunks,  CervinizatorGraphics::kKnobOutOvNumChunks);
    customLookAndFeel.bypassOffImg  = loadEmbeddedImage(CervinizatorGraphics::kBypassOffChunks,  CervinizatorGraphics::kBypassOffNumChunks);
    customLookAndFeel.bypassOnImg   = loadEmbeddedImage(CervinizatorGraphics::kBypassOnChunks,   CervinizatorGraphics::kBypassOnNumChunks);
    customLookAndFeel.limitOffImg   = loadEmbeddedImage(CervinizatorGraphics::kLimitOffChunks,   CervinizatorGraphics::kLimitOffNumChunks);
    customLookAndFeel.limitOnImg    = loadEmbeddedImage(CervinizatorGraphics::kLimitOnChunks,    CervinizatorGraphics::kLimitOnNumChunks);

    setLookAndFeel(&customLookAndFeel);
    setSize(480, 520);

    // 1. Top-Left Knob: INPUT (-24 .. +24 dB, default 0.0 dB, Shift+LMB = Auto-Calibration)
    inputSlider.setName("INPUT");
    inputSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    inputSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    inputSlider.setDoubleClickReturnValue(true, 0.0);
    inputSlider.onShiftClick = [this]()
    {
        audioProcessor.startInputCalibration();
        calibCompleteHoldTicks = 0;
        repaint();
    };
    addAndMakeVisible(inputSlider);

    // 2. Top-Right Knob: OUTPUT (-24 .. +24 dB, default 0.0 dB)
    outputSlider.setName("OUTPUT");
    outputSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    outputSlider.setDoubleClickReturnValue(true, 0.0);
    addAndMakeVisible(outputSlider);

    // 3. Center Main One-Knob: CERVINIZE (0% .. 100%, default 0.0%)
    enhanceSlider.setName("ENHANCE");
    enhanceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    enhanceSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    enhanceSlider.setDoubleClickReturnValue(true, 0.0);
    enhanceSlider.onValueChange = [this]() { repaint(); };
    addAndMakeVisible(enhanceSlider);

    // 4. Bottom-Left Button: BYPASS
    bypassButton.setName("BYPASS");
    bypassButton.setButtonText("");
    addAndMakeVisible(bypassButton);

    // 5. Bottom-Right Button: LIMIT
    limitButton.setName("LIMIT");
    limitButton.setButtonText("");
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

void CervinizatorAudioProcessorEditor::mouseDown(const juce::MouseEvent& /*e*/)
{
    if (audioProcessor.getCalibrationState() != CervinizatorAudioProcessor::CalibState::Idle)
    {
        audioProcessor.cancelInputCalibration();
        repaint();
    }
}

void CervinizatorAudioProcessorEditor::timerCallback()
{
    // 1. Apply any new calibrated INPUT gain computed by the audio thread
    float newCalibDb = 0.0f;
    if (audioProcessor.consumePendingCalibDb(newCalibDb))
    {
        inputSlider.setValue(newCalibDb, juce::sendNotificationSync);
    }

    const auto cState = audioProcessor.getCalibrationState();
    if (cState == CervinizatorAudioProcessor::CalibState::Complete)
    {
        if (++calibCompleteHoldTicks >= 40) // Hold completion message for ~1.3s then close
        {
            audioProcessor.cancelInputCalibration();
            calibCompleteHoldTicks = 0;
        }
        repaint();
        return;
    }
    else if (cState != CervinizatorAudioProcessor::CalibState::Idle)
    {
        hoveredControlId = -1;
        hoverTicks = 0;
        activeTooltipText.clear();
        repaint();
        return;
    }

    // 2. Hover Tooltip detection (~650 ms = 20 ticks at 30 Hz without mouse button pressed)
    const auto mousePos = getMouseXYRelative();
    const bool mouseDownNow = juce::ModifierKeys::currentModifiers.isAnyMouseButtonDown();

    int newHoverId = -1;
    juce::String tipText;
    juce::Rectangle<float> targetRect;

    if (!mouseDownNow && isMouseOver(true))
    {
        if (enhanceSlider.getBounds().contains(mousePos))
        {
            newHoverId = 1;
            tipText = "CERVINIZATION - The more you turn this knob, the more your drums fly into Octane playlists.";
            targetRect = enhanceSlider.getBounds().toFloat();
        }
        else if (inputSlider.getBounds().contains(mousePos))
        {
            newHoverId = 2;
            tipText = "SHIFT+LMB to calibrate input signal. Use this if nature didn't bless you with ears.";
            targetRect = inputSlider.getBounds().toFloat();
        }
        else if (outputSlider.getBounds().contains(mousePos))
        {
            newHoverId = 3;
            tipText = "Output volume of drums processed by the greatest plugin in the world.";
            targetRect = outputSlider.getBounds().toFloat();
        }
        else if (bypassButton.getBounds().contains(mousePos))
        {
            newHoverId = 4;
            tipText = "The truest, most genuine, sincere, and unbelievable Bypass in the world.";
            targetRect = bypassButton.getBounds().toFloat();
        }
        else if (limitButton.getBounds().contains(mousePos))
        {
            newHoverId = 5;
            tipText = "I'm actually a clipper, but you won't tell the difference anyway.";
            targetRect = limitButton.getBounds().toFloat();
        }
    }

    if (newHoverId != -1 && newHoverId == hoveredControlId)
    {
        if (hoverTicks < 100)
            ++hoverTicks;
        if (hoverTicks == 20) // ~650 ms hover threshold reached
        {
            activeTooltipText = tipText;
            activeTooltipTargetBounds = targetRect;
            repaint();
        }
    }
    else
    {
        const bool wasShowing = activeTooltipText.isNotEmpty();
        hoveredControlId = newHoverId;
        hoverTicks = 0;
        activeTooltipText.clear();
        if (wasShowing)
            repaint();
    }
}

void CervinizatorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

    const auto bounds = getLocalBounds().toFloat();

    // 1. Draw base background (0% position)
    if (bg0Image.isValid())
        g.drawImage(bg0Image, bounds);
    else
        g.fillAll(juce::Colour(0xff141820));

    // 2. Smoothly crossfade background_100 proportionally to the main CERVINIZE knob (0% -> 100%)
    const float main01 = juce::jlimit(0.0f, 1.0f, static_cast<float>(enhanceSlider.getValue()) * 0.01f);
    if (bg100Image.isValid() && main01 > 0.001f)
    {
        g.setOpacity(main01);
        g.drawImage(bg100Image, bounds);
        g.setOpacity(1.0f);
    }
}

void CervinizatorAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    // 1. Draw Shift + LMB Auto-Calibration Popup Window when active
    const auto cState = audioProcessor.getCalibrationState();
    if (cState != CervinizatorAudioProcessor::CalibState::Idle)
    {
        g.setColour(juce::Colours::black.withAlpha(0.62f));
        g.fillRect(getLocalBounds());

        const juce::Rectangle<float> box(55.0f, 195.0f, 370.0f, 130.0f);
        g.setColour(juce::Colour(0xff111621).withAlpha(0.96f));
        g.fillRoundedRectangle(box, 10.0f);

        g.setColour(juce::Colour(0xffef4444));
        g.drawRoundedRectangle(box, 10.0f, 2.0f);

        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(15.0f, juce::Font::bold));

        if (cState == CervinizatorAudioProcessor::CalibState::WaitingForAudio)
        {
            g.drawText("INPUT AUTO-CALIBRATION", box.withHeight(38.0f).translated(0.0f, 10.0f),
                       juce::Justification::centred, false);

            g.setColour(juce::Colour(0xffe2e8f0));
            g.setFont(juce::Font(13.5f, juce::Font::plain));
            g.drawFittedText("Play your drums audio so we can calibrate the optimal input volume...",
                             box.reduced(22.0f, 0.0f).withTrimmedTop(44.0f).withHeight(50.0f).toNearestInt(),
                             juce::Justification::centred, 2);

            g.setColour(juce::Colour(0xff94a3b8));
            g.setFont(juce::Font(11.0f, juce::Font::plain));
            g.drawText("(Click anywhere to cancel)", box.withTrimmedTop(96.0f),
                       juce::Justification::centred, false);
        }
        else if (cState == CervinizatorAudioProcessor::CalibState::Analyzing)
        {
            const float prog = audioProcessor.getCalibrationProgress();
            g.drawText("CALIBRATING INPUT LEVEL...", box.withHeight(36.0f).translated(0.0f, 10.0f),
                       juce::Justification::centred, false);

            g.setColour(juce::Colour(0xffe2e8f0));
            g.setFont(juce::Font(13.0f, juce::Font::plain));
            g.drawText("Analyzing drum transients (" + juce::String(static_cast<int>(prog * 100.0f)) + "%)",
                       box.withTrimmedTop(42.0f).withHeight(28.0f),
                       juce::Justification::centred, false);

            // Progress bar
            const juce::Rectangle<float> barBg(box.getX() + 35.0f, box.getY() + 80.0f, box.getWidth() - 70.0f, 14.0f);
            g.setColour(juce::Colour(0xff1e293b));
            g.fillRoundedRectangle(barBg, 6.0f);

            g.setColour(juce::Colour(0xffef4444));
            g.fillRoundedRectangle(barBg.withWidth(barBg.getWidth() * prog), 6.0f);
        }
        else if (cState == CervinizatorAudioProcessor::CalibState::Complete)
        {
            const float finalDb = audioProcessor.getCalibratedInputDb();
            const juce::String sign = (finalDb >= 0.0f) ? "+" : "";
            g.setColour(juce::Colour(0xff22c55e));
            g.drawText("CALIBRATION COMPLETE!", box.withHeight(42.0f).translated(0.0f, 16.0f),
                       juce::Justification::centred, false);

            g.setColour(juce::Colours::white);
            g.setFont(juce::Font(14.0f, juce::Font::bold));
            g.drawText("Optimal Input set to " + sign + juce::String(finalDb, 1) + " dB",
                       box.withTrimmedTop(58.0f).withHeight(40.0f),
                       juce::Justification::centred, false);
        }
        return;
    }

    // 2. Draw Delayed Hover Tooltip Card
    if (activeTooltipText.isNotEmpty())
    {
        const float tipW = 250.0f;
        const float tipH = 54.0f;
        float tipX = activeTooltipTargetBounds.getCentreX() - tipW * 0.5f;
        float tipY = activeTooltipTargetBounds.getY() - tipH - 8.0f;

        // Keep tooltip inside window bounds
        tipX = juce::jlimit(10.0f, static_cast<float>(getWidth()) - tipW - 10.0f, tipX);
        if (tipY < 10.0f)
            tipY = activeTooltipTargetBounds.getBottom() + 8.0f;

        const juce::Rectangle<float> tipBox(tipX, tipY, tipW, tipH);

        g.setColour(juce::Colour(0xff0f141d).withAlpha(0.95f));
        g.fillRoundedRectangle(tipBox, 7.0f);

        g.setColour(juce::Colour(0xffef4444).withAlpha(0.85f));
        g.drawRoundedRectangle(tipBox, 7.0f, 1.5f);

        g.setColour(juce::Colour(0xfff1f5f9));
        g.setFont(juce::Font(12.0f, juce::Font::plain));
        g.drawFittedText(activeTooltipText,
                         tipBox.reduced(10.0f, 6.0f).toNearestInt(),
                         juce::Justification::centred, 3);
    }
}

void CervinizatorAudioProcessorEditor::resized()
{
    // Exact 1x coordinates corresponding to the 2x (960x1040) Photoshop layers:
    // Input small knob: 2x crop (80, 154, 144x144) -> 1x (40, 77, 72x72)
    inputSlider.setBounds(40, 77, 72, 72);

    // Output small knob: 2x crop (736, 154, 144x144) -> 1x (368, 77, 72x72)
    outputSlider.setBounds(368, 77, 72, 72);

    // Center Main Knob: 2x crop (271, 334, 416x416) -> 1x (136, 167, 208x208)
    enhanceSlider.setBounds(136, 167, 208, 208);

    // Bottom-Left Bypass Button: 2x crop (140, 864, 144x124) -> 1x (70, 432, 72x62)
    bypassButton.setBounds(70, 432, 72, 62);

    // Bottom-Right Limit Button: 2x crop (610, 864, 136x124) -> 1x (305, 432, 68x62)
    limitButton.setBounds(305, 432, 68, 62);
}
