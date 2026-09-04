#pragma once

#include "LouderEngine.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

namespace g3x::louder
{
class LouderAudioProcessor final : public juce::AudioProcessor
{
public:
    LouderAudioProcessor();
    using juce::AudioProcessor::processBlock;
    void prepareToPlay(double, int) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState& getParameters() noexcept { return state; }
    float getInputPeak() const noexcept { return engine.getInputPeak(); }
    float getOutputPeak() const noexcept { return engine.getOutputPeak(); }
    float getUpwardGainDb() const noexcept { return engine.getUpwardGainDb(); }
    float getLimiterReductionDb() const noexcept { return engine.getLimiterReductionDb(); }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateEngine() noexcept;
    juce::AudioProcessorValueTreeState state;
    LouderEngine engine;
    std::atomic<int> currentProgram { 0 };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LouderAudioProcessor)
};
}
