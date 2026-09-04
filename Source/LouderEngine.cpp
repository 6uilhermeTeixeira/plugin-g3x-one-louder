#include "LouderEngine.h"
#include <algorithm>
#include <cmath>

namespace g3x::louder
{
void LouderEngine::Smoother::prepare(double rate, double seconds, float value) noexcept
{
    rampSamples = std::max(1, static_cast<int>(rate * seconds));
    current = target = value;
    remaining = 0;
    step = 0.0f;
}

void LouderEngine::Smoother::setTarget(float value) noexcept
{
    if (!std::isfinite(value)) value = 0.0f;
    if (std::abs(value - target) <= 1.0e-7f) return;
    target = value;
    remaining = rampSamples;
    step = (target - current) / static_cast<float>(remaining);
}

float LouderEngine::Smoother::next() noexcept
{
    if (remaining > 0)
    {
        current += step;
        if (--remaining == 0) current = target;
    }
    return current;
}

void LouderEngine::prepare(double newSampleRate, int channels)
{
    sampleRate = std::isfinite(newSampleRate) ? std::max(1.0, newSampleRate) : 44100.0;
    delaySamples = std::max(1, static_cast<int>(std::ceil(sampleRate * 0.001)));
    const auto channelsToPrepare = std::clamp(channels, 1, maximumChannels);
    for (int channel = 0; channel < maximumChannels; ++channel)
    {
        const auto size = channel < channelsToPrepare ? static_cast<std::size_t>(delaySamples) : 0U;
        wetDelay[static_cast<std::size_t>(channel)].assign(size, 0.0f);
        dryDelay[static_cast<std::size_t>(channel)].assign(size, 0.0f);
    }
    detector.prepare(sampleRate);
    amount.prepare(sampleRate, 0.02, parameters.amount);
    activeMix.prepare(sampleRate, 0.01, parameters.bypass ? 0.0f : 1.0f);
    reset();
}

void LouderEngine::reset() noexcept
{
    for (auto& delay : wetDelay) std::fill(delay.begin(), delay.end(), 0.0f);
    for (auto& delay : dryDelay) std::fill(delay.begin(), delay.end(), 0.0f);
    detector.reset();
    writeIndex = 0;
    limiterGain = 1.0f;
    inputPeak.store(0.0f, std::memory_order_relaxed);
    outputPeak.store(0.0f, std::memory_order_relaxed);
    upwardGainDb.store(0.0f, std::memory_order_relaxed);
    limiterReductionDb.store(0.0f, std::memory_order_relaxed);
}

void LouderEngine::setParameters(const EngineParameters& values) noexcept
{
    parameters = values;
    amount.setTarget(std::clamp(std::isfinite(values.amount) ? values.amount : 0.0f,
                                0.0f, 10.0f));
    activeMix.setTarget(values.bypass ? 0.0f : 1.0f);
}

void LouderEngine::process(float* const* channels, int channelCount, int sampleCount) noexcept
{
    const auto count = std::clamp(channelCount, 0, maximumChannels);
    auto blockInputPeak = 0.0f;
    auto blockOutputPeak = 0.0f;
    float lastUpwardDb = 0.0f;
    for (int sample = 0; sample < std::max(0, sampleCount); ++sample)
    {
        std::array<float, maximumChannels> input {};
        auto linkedMagnitude = 0.0f;
        for (int channel = 0; channel < count; ++channel)
        {
            const auto raw = channels[channel][sample];
            input[static_cast<std::size_t>(channel)] = std::isfinite(raw) ? raw : 0.0f;
            linkedMagnitude = std::max(linkedMagnitude,
                std::abs(input[static_cast<std::size_t>(channel)]));
        }
        const auto settings = dsp::mapAmount(amount.next());
        const auto levelDb = dsp::gainToDecibels(detector.process(linkedMagnitude));
        lastUpwardDb = dsp::computeUpwardGainDb(levelDb, settings);
        const auto dynamicsGain = dsp::decibelsToGain(settings.makeupDb + lastUpwardDb);
        const auto ceiling = parameters.ceilingMode == CeilingMode::safe
            ? dsp::decibelsToGain(-1.0f) : dsp::decibelsToGain(-0.1f);
        const auto predictedPeak = linkedMagnitude * dynamicsGain;
        const auto desiredLimiterGain = predictedPeak > ceiling
            ? ceiling / predictedPeak : 1.0f;
        if (desiredLimiterGain < limiterGain)
            limiterGain = desiredLimiterGain;
        else
        {
            const auto reductionDb = -dsp::gainToDecibels(std::max(limiterGain, 1.0e-6f));
            const auto releaseMs = std::clamp(70.0f + reductionDb * 10.0f, 70.0f, 250.0f);
            const auto coefficient = std::exp(-1.0f / (0.001f * releaseMs
                * static_cast<float>(sampleRate)));
            limiterGain = 1.0f + coefficient * (limiterGain - 1.0f);
        }
        const auto mix = activeMix.next();
        for (int channel = 0; channel < count; ++channel)
        {
            const auto index = static_cast<std::size_t>(channel);
            const auto delayedWet = wetDelay[index][static_cast<std::size_t>(writeIndex)];
            const auto delayedDry = dryDelay[index][static_cast<std::size_t>(writeIndex)];
            wetDelay[index][static_cast<std::size_t>(writeIndex)] = input[index] * dynamicsGain;
            dryDelay[index][static_cast<std::size_t>(writeIndex)] = input[index];
            const auto wet = delayedWet * limiterGain;
            const auto output = delayedDry + mix * (wet - delayedDry);
            channels[channel][sample] = std::isfinite(output) ? output : 0.0f;
            blockInputPeak = std::max(blockInputPeak, std::abs(input[index]));
            blockOutputPeak = std::max(blockOutputPeak, std::abs(output));
        }
        if (++writeIndex >= delaySamples) writeIndex = 0;
    }
    inputPeak.store(blockInputPeak, std::memory_order_relaxed);
    outputPeak.store(blockOutputPeak, std::memory_order_relaxed);
    upwardGainDb.store(lastUpwardDb, std::memory_order_relaxed);
    limiterReductionDb.store(dsp::gainToDecibels(limiterGain), std::memory_order_relaxed);
}
}
