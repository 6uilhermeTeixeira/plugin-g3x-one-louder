#pragma once
#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

namespace g3x::louder
{
class LouderLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    LouderLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float,
                          juce::Slider&) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool, bool) override;
};

class Meter final : public juce::Component
{
public:
    enum class Kind { level, upward, reduction };
    Meter(Kind, juce::String);
    void setDecibels(float);
    void paint(juce::Graphics&) override;
private:
    Kind kind;
    juce::String label;
    float value = -120.0f;
};

class LouderAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit LouderAudioProcessorEditor(LouderAudioProcessor&);
    ~LouderAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    LouderAudioProcessor& processor;
    LouderLookAndFeel lookAndFeel;
    juce::Slider amountSlider;
    juce::ComboBox ceilingBox, presetBox;
    juce::ToggleButton bypassButton { "BYPASS" };
    juce::Label latencyLabel;
    Meter inputMeter { Meter::Kind::level, "IN" };
    Meter upwardMeter { Meter::Kind::upward, "LIFT" };
    Meter limiterMeter { Meter::Kind::reduction, "LIMIT" };
    Meter outputMeter { Meter::Kind::level, "OUT" };
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> amountAttachment;
    std::unique_ptr<ComboAttachment> ceilingAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LouderAudioProcessorEditor)
};
}
