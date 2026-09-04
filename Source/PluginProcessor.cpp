#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <array>

namespace g3x::louder
{
namespace Ids
{
constexpr auto amount = "amount";
constexpr auto ceiling = "ceilingMode";
constexpr auto bypass = "bypass";
}

struct FactoryPreset { const char* name; float amount; CeilingMode ceiling; };
constexpr std::array<FactoryPreset, 6> factoryPresets {{
    { "Neutral", 0.0f, CeilingMode::safe },
    { "Vocal Lift", 3.2f, CeilingMode::safe },
    { "Drum Power", 5.8f, CeilingMode::hot },
    { "Bus Density", 4.6f, CeilingMode::safe },
    { "Mix Safe", 2.8f, CeilingMode::safe },
    { "Maximum Impact", 8.5f, CeilingMode::hot },
}};

LouderAudioProcessor::LouderAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "G3XLouderState", createParameterLayout())
{
}

void LouderAudioProcessor::prepareToPlay(double sampleRate, int)
{
    engine.prepare(sampleRate, getTotalNumOutputChannels());
    setLatencySamples(engine.getLatencySamples());
    updateEngine();
}

void LouderAudioProcessor::releaseResources() { engine.reset(); }

bool LouderAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    return (input == juce::AudioChannelSet::mono() || input == juce::AudioChannelSet::stereo())
        && input == layouts.getMainOutputChannelSet();
}

void LouderAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    updateEngine();
    engine.process(buffer.getArrayOfWritePointers(), buffer.getNumChannels(), buffer.getNumSamples());
}

juce::AudioProcessorEditor* LouderAudioProcessor::createEditor()
{
    return new LouderAudioProcessorEditor(*this);
}

int LouderAudioProcessor::getNumPrograms() { return static_cast<int>(factoryPresets.size()); }
int LouderAudioProcessor::getCurrentProgram() { return currentProgram.load(); }
const juce::String LouderAudioProcessor::getProgramName(int index)
{
    return juce::isPositiveAndBelow(index, getNumPrograms())
        ? factoryPresets[static_cast<std::size_t>(index)].name : juce::String {};
}

void LouderAudioProcessor::setCurrentProgram(int index)
{
    if (!juce::isPositiveAndBelow(index, getNumPrograms())) return;
    const auto& preset = factoryPresets[static_cast<std::size_t>(index)];
    const auto set = [this](const char* id, float value)
    {
        if (auto* parameter = state.getParameter(id))
        {
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
            parameter->endChangeGesture();
        }
    };
    set(Ids::amount, preset.amount);
    set(Ids::ceiling, preset.ceiling == CeilingMode::safe ? 0.0f : 1.0f);
    set(Ids::bypass, 0.0f);
    currentProgram.store(index);
}

void LouderAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    auto saved = state.copyState();
    saved.setProperty("schemaVersion", 1, nullptr);
    saved.setProperty("currentProgram", currentProgram.load(), nullptr);
    if (const auto xml = saved.createXml()) copyXmlToBinary(*xml, destination);
}

void LouderAudioProcessor::setStateInformation(const void* data, int size)
{
    if (const auto xml = getXmlFromBinary(data, size); xml != nullptr)
        if (xml->hasTagName(state.state.getType()))
        {
            auto restored = juce::ValueTree::fromXml(*xml);
            currentProgram.store(juce::jlimit(0, getNumPrograms() - 1,
                static_cast<int>(restored.getProperty("currentProgram", 0))));
            state.replaceState(restored);
        }
}

juce::AudioProcessorValueTreeState::ParameterLayout LouderAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { Ids::amount, 1 }, "Louder",
        juce::NormalisableRange<float> { 0.0f, 10.0f, 0.01f }, 0.0f,
        juce::AudioParameterFloatAttributes {}.withStringFromValueFunction(
            [](float value, int) { return juce::String(value, 1); })));
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { Ids::ceiling, 1 }, "Ceiling",
        juce::StringArray { "Safe (-1 dB)", "Hot (-0.1 dB)" }, 0));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { Ids::bypass, 1 }, "Bypass", false));
    return { parameters.begin(), parameters.end() };
}

void LouderAudioProcessor::updateEngine() noexcept
{
    EngineParameters values;
    values.amount = state.getRawParameterValue(Ids::amount)->load();
    values.ceilingMode = state.getRawParameterValue(Ids::ceiling)->load() < 0.5f
        ? CeilingMode::safe : CeilingMode::hot;
    values.bypass = state.getRawParameterValue(Ids::bypass)->load() >= 0.5f;
    engine.setParameters(values);
}
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new g3x::louder::LouderAudioProcessor();
}
