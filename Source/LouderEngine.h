#pragma once

#include "dsp/LouderDynamics.h"
#include <array>
#include <atomic>
#include <vector>

namespace g3x::louder
{
enum class CeilingMode { safe, hot };

struct EngineParameters
{
    float amount = 0.0f;
    CeilingMode ceilingMode = CeilingMode::safe;
    bool bypass = false;
};

class LouderEngine
{
public:
    void prepare(double sampleRate, int channels);
    void reset() noexcept;
    void setParameters(const EngineParameters&) noexcept;
    void process(float* const* channels, int channelCount, int sampleCount) noexcept;
    int getLatencySamples() const noexcept { return delaySamples; }
    float getInputPeak() const noexcept { return inputPeak.load(std::memory_order_relaxed); }
    float getOutputPeak() const noexcept { return outputPeak.load(std::memory_order_relaxed); }
    float getUpwardGainDb() const noexcept { return upwardGainDb.load(std::memory_order_relaxed); }
    float getLimiterReductionDb() const noexcept
    {
        return limiterReductionDb.load(std::memory_order_relaxed);
    }

private:
    struct Smoother
    {
        void prepare(double rate, double seconds, float value) noexcept;
        void setTarget(float value) noexcept;
        float next() noexcept;
        float current = 0.0f, target = 0.0f, step = 0.0f;
        int remaining = 0, rampSamples = 1;
    };

    static constexpr int maximumChannels = 2;
    std::array<std::vector<float>, maximumChannels> wetDelay;
    std::array<std::vector<float>, maximumChannels> dryDelay;
    dsp::RmsDetector detector;
    Smoother amount;
    Smoother activeMix;
    EngineParameters parameters;
    double sampleRate = 44100.0;
    int delaySamples = 44;
    int writeIndex = 0;
    float limiterGain = 1.0f;
    std::atomic<float> inputPeak { 0.0f };
    std::atomic<float> outputPeak { 0.0f };
    std::atomic<float> upwardGainDb { 0.0f };
    std::atomic<float> limiterReductionDb { 0.0f };
};
}
