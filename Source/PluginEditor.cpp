#include "PluginEditor.h"
#include <cmath>

namespace g3x::louder
{
namespace C
{
const auto bg = juce::Colour::fromRGB(9, 20, 18), panel = juce::Colour::fromRGB(16, 38, 32);
const auto edge = juce::Colour::fromRGB(45, 83, 69), green = juce::Colour::fromRGB(67, 224, 143);
const auto amber = juce::Colour::fromRGB(255, 174, 70), red = juce::Colour::fromRGB(255, 91, 76);
const auto text = juce::Colour::fromRGB(235, 247, 240), muted = juce::Colour::fromRGB(130, 164, 150);
}

LouderLookAndFeel::LouderLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, C::text);
    setColour(juce::Slider::textBoxBackgroundColourId, C::bg);
    setColour(juce::Slider::textBoxOutlineColourId, C::edge);
    setColour(juce::ComboBox::backgroundColourId, C::bg);
    setColour(juce::ComboBox::textColourId, C::text);
    setColour(juce::ComboBox::outlineColourId, C::edge);
    setColour(juce::ComboBox::arrowColourId, C::green);
    setColour(juce::PopupMenu::backgroundColourId, C::panel);
    setColour(juce::PopupMenu::textColourId, C::text);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, C::green.darker(0.5f));
}

void LouderLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                         float p, float start, float end, juce::Slider& slider)
{
    const auto r = 0.5f * static_cast<float>(std::min(w, h)) - 14.0f;
    const juce::Point<float> centre { static_cast<float>(x + w / 2), static_cast<float>(y + h / 2) };
    const auto circle = juce::Rectangle<float>(2.0f * r, 2.0f * r).withCentre(centre);
    g.setColour(C::bg); g.fillEllipse(circle); g.setColour(C::edge); g.drawEllipse(circle, 2.0f);
    const auto angle = start + p * (end - start);
    juce::Path arc;
    arc.addCentredArc(centre.x, centre.y, r + 7.0f, r + 7.0f, 0.0f, start, angle, true);
    g.setGradientFill({ C::green, circle.getX(), circle.getBottom(), C::amber,
                        circle.getRight(), circle.getY(), false });
    g.strokePath(arc, juce::PathStrokeType(9.0f, juce::PathStrokeType::curved,
                                           juce::PathStrokeType::rounded));
    juce::Path pointer; pointer.addRoundedRectangle(-3.0f, -r + 18.0f, 6.0f, r * 0.5f, 3.0f);
    g.setColour(C::amber); g.fillPath(pointer,
        juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
    if (slider.hasKeyboardFocus(false)) { g.setColour(C::text); g.drawEllipse(circle.expanded(15.0f), 1.5f); }
}

void LouderLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool over, bool)
{
    const auto bounds = b.getLocalBounds().toFloat().reduced(1.0f);
    g.setColour(b.getToggleState() ? C::red.darker(0.25f) : C::bg); g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(b.getToggleState() ? C::red : C::edge.brighter(over ? 0.2f : 0.0f));
    g.drawRoundedRectangle(bounds, 6.0f, 1.5f); g.setColour(C::text);
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.drawText(b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
}

Meter::Meter(Kind k, juce::String name) : kind(k), label(std::move(name))
{
    setTitle(label + " meter"); setDescription("Real-time audio processing meter");
    setInterceptsMouseClicks(false, false);
}
void Meter::setDecibels(float db) { value = std::isfinite(db) ? db : -120.0f; repaint(); }
void Meter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds(); const auto caption = bounds.removeFromBottom(17);
    auto track = bounds.toFloat().reduced(4.0f, 2.0f); g.setColour(C::bg);
    g.fillRoundedRectangle(track, 4.0f); g.setColour(C::edge); g.drawRoundedRectangle(track, 4.0f, 1.0f);
    const auto n = kind == Kind::level ? juce::jlimit(0.0f, 1.0f, (value + 60.0f) / 60.0f)
                 : juce::jlimit(0.0f, 1.0f, std::abs(value) / (kind == Kind::upward ? 12.0f : 24.0f));
    auto fill = track.reduced(3.0f); fill.removeFromTop(fill.getHeight() * (1.0f - n));
    g.setColour(kind == Kind::reduction ? C::red : kind == Kind::upward ? C::amber : C::green);
    g.fillRoundedRectangle(fill, 2.0f); g.setColour(C::muted); g.setFont(juce::FontOptions(9.0f));
    g.drawText(label, caption, juce::Justification::centred);
}

LouderAudioProcessorEditor::LouderAudioProcessorEditor(LouderAudioProcessor& owner)
    : AudioProcessorEditor(owner), processor(owner)
{
    setLookAndFeel(&lookAndFeel); setResizable(true, true); setResizeLimits(480, 440, 820, 760); setSize(560, 560);
    amountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    amountSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 29);
    amountSlider.setDoubleClickReturnValue(true, 0.0); amountSlider.setTitle("Louder amount");
    amountSlider.setDescription("Controls low-level compression, makeup and limiting from zero to ten");
    ceilingBox.addItemList({ "SAFE  -1 dB", "HOT  -0.1 dB" }, 1);
    ceilingBox.setTitle("Output ceiling"); ceilingBox.setDescription("Selects conservative or hot peak ceiling");
    bypassButton.setTitle("Plugin bypass");
    for (int i = 0; i < processor.getNumPrograms(); ++i) presetBox.addItem(processor.getProgramName(i), i + 1);
    presetBox.setSelectedItemIndex(processor.getCurrentProgram(), juce::dontSendNotification);
    presetBox.setTitle("Factory preset"); presetBox.onChange = [this] { processor.setCurrentProgram(presetBox.getSelectedItemIndex()); };
    latencyLabel.setText("LOOKAHEAD  1.0 ms", juce::dontSendNotification);
    latencyLabel.setJustificationType(juce::Justification::centred); latencyLabel.setColour(juce::Label::textColourId, C::muted);
    for (auto* c : std::initializer_list<juce::Component*> { &amountSlider, &ceilingBox, &presetBox, &bypassButton,
             &latencyLabel, &inputMeter, &upwardMeter, &limiterMeter, &outputMeter }) addAndMakeVisible(c);
    auto& state = processor.getParameters();
    amountAttachment = std::make_unique<SliderAttachment>(state, "amount", amountSlider);
    ceilingAttachment = std::make_unique<ComboAttachment>(state, "ceilingMode", ceilingBox);
    bypassAttachment = std::make_unique<ButtonAttachment>(state, "bypass", bypassButton);
    startTimerHz(40);
}
LouderAudioProcessorEditor::~LouderAudioProcessorEditor() { stopTimer(); setLookAndFeel(nullptr); }

void LouderAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.setGradientFill({ C::panel.brighter(0.06f), 0.0f, 0.0f, C::bg, 0.0f, static_cast<float>(getHeight()), false }); g.fillAll();
    auto header = getLocalBounds().removeFromTop(78); g.setColour(C::green); g.fillRect(header.removeFromBottom(2));
    g.setColour(C::text); g.setFont(juce::FontOptions(25.0f, juce::Font::bold));
    g.drawText("G3X", header.reduced(22, 8).removeFromLeft(68), juce::Justification::centredLeft);
    g.setColour(C::amber); g.setFont(juce::FontOptions(17.0f));
    g.drawText("LOUDER", header.reduced(88, 8), juce::Justification::centredLeft);
}

void LouderAudioProcessorEditor::resized()
{
    auto header = getLocalBounds().removeFromTop(78).reduced(16, 17); bypassButton.setBounds(header.removeFromRight(78));
    header.removeFromRight(8); presetBox.setBounds(header.removeFromRight(std::min(220, header.getWidth())));
    auto body = getLocalBounds().withTrimmedTop(92).withTrimmedBottom(18).reduced(20, 0);
    auto footer = body.removeFromBottom(48); ceilingBox.setBounds(footer.removeFromLeft(150).reduced(4, 7));
    latencyLabel.setBounds(footer.reduced(5, 7)); auto meters = body.removeFromRight(150).reduced(4, 10);
    const auto width = meters.getWidth() / 4; inputMeter.setBounds(meters.removeFromLeft(width));
    upwardMeter.setBounds(meters.removeFromLeft(width)); limiterMeter.setBounds(meters.removeFromLeft(width));
    outputMeter.setBounds(meters); amountSlider.setBounds(body.reduced(6, 2));
}

void LouderAudioProcessorEditor::timerCallback()
{
    inputMeter.setDecibels(dsp::gainToDecibels(processor.getInputPeak()));
    outputMeter.setDecibels(dsp::gainToDecibels(processor.getOutputPeak()));
    upwardMeter.setDecibels(processor.getUpwardGainDb());
    limiterMeter.setDecibels(processor.getLimiterReductionDb());
}
}
