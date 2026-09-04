#include "LouderDynamics.h"
#include <algorithm>
#include <cmath>

namespace g3x::louder::dsp
{
MacroSettings mapAmount(float amount) noexcept
{
    const auto safe = std::isfinite(amount) ? amount : 0.0f;
    const auto normalized = std::clamp(safe * 0.1f, 0.0f, 1.0f);
    const auto shaped = std::pow(normalized, 1.45f);
    return {
        18.0f * shaped,
        12.0f * std::pow(normalized, 1.25f),
        1.0f + 3.0f * normalized,
        -28.0f - 8.0f * normalized,
        -70.0f
    };
}

float gainToDecibels(float gain) noexcept
{
    return std::isfinite(gain) && gain > 0.0f
        ? std::max(minimumDecibels, 20.0f * std::log10(gain)) : minimumDecibels;
}

float decibelsToGain(float decibels) noexcept
{
    return std::isfinite(decibels) && decibels > minimumDecibels
        ? std::pow(10.0f, decibels / 20.0f) : 0.0f;
}

float computeUpwardGainDb(float levelDb, const MacroSettings& settings) noexcept
{
    if (!std::isfinite(levelDb) || levelDb >= settings.thresholdDb
        || settings.maximumUpwardDb <= 0.0f || settings.ratio <= 1.0f)
        return 0.0f;
    const auto gateWidthDb = 12.0f;
    const auto activity = std::clamp((levelDb - settings.activityFloorDb) / gateWidthDb,
                                     0.0f, 1.0f);
    const auto smoothActivity = activity * activity * (3.0f - 2.0f * activity);
    const auto compression = (settings.thresholdDb - levelDb)
                           * (1.0f - 1.0f / settings.ratio);
    return std::min(settings.maximumUpwardDb, std::max(0.0f, compression)) * smoothActivity;
}

void RmsDetector::prepare(double newSampleRate) noexcept
{
    sampleRate = std::isfinite(newSampleRate) ? std::max(1.0, newSampleRate) : 44100.0;
    const auto coefficient = [this](float milliseconds)
    {
        return std::exp(-1.0f / (0.001f * milliseconds * static_cast<float>(sampleRate)));
    };
    attackCoefficient = coefficient(10.0f);
    releaseCoefficient = coefficient(120.0f);
    reset();
}

float RmsDetector::process(float linkedMagnitude) noexcept
{
    const auto magnitude = std::isfinite(linkedMagnitude) ? std::abs(linkedMagnitude) : 0.0f;
    const auto power = magnitude * magnitude;
    const auto coefficient = power > meanSquare ? attackCoefficient : releaseCoefficient;
    meanSquare = power + coefficient * (meanSquare - power);
    return std::sqrt(std::max(0.0f, meanSquare));
}
}
