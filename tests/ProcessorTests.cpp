#include "PluginProcessor.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string_view>

namespace
{
int failures = 0;
void expect(bool condition, std::string_view message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
}

int main()
{
    using g3x::louder::LouderAudioProcessor;
    LouderAudioProcessor source;
    source.prepareToPlay(48000.0, 64);
    expect(source.getLatencySamples() == 48, "processor reports one millisecond at 48 kHz");
    expect(source.getNumPrograms() == 6, "six presets are exposed");
    expect(source.getProgramName(0) == "Neutral", "first preset is Neutral");
    expect(source.getProgramName(5) == "Maximum Impact", "last preset is Maximum Impact");

    source.setCurrentProgram(5);
    const auto amount = source.getParameters().getRawParameterValue("amount")->load();
    const auto ceiling = source.getParameters().getRawParameterValue("ceilingMode")->load();
    expect(std::abs(amount - 8.5f) < 0.001f, "Maximum Impact recalls amount");
    expect(ceiling > 0.5f, "Maximum Impact recalls Hot ceiling");

    juce::MemoryBlock state;
    source.getStateInformation(state);
    LouderAudioProcessor restored;
    restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    expect(std::abs(restored.getParameters().getRawParameterValue("amount")->load() - amount) < 0.001f,
           "amount survives state round-trip");
    expect(restored.getCurrentProgram() == 5, "preset index survives state round-trip");

    juce::AudioBuffer<float> audio(2, 64);
    audio.clear();
    juce::MidiBuffer midi;
    source.processBlock(audio, midi);
    bool finite = true;
    for (int channel = 0; channel < audio.getNumChannels(); ++channel)
        for (int sample = 0; sample < audio.getNumSamples(); ++sample)
            finite = finite && std::isfinite(audio.getSample(channel, sample));
    expect(finite, "processor produces finite silence");

    if (failures == 0) std::cout << "All G3X Louder processor tests passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
