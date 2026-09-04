#pragma once

namespace g3x::louder::dsp
{
constexpr float minimumDecibels = -120.0f;

struct MacroSettings
{
    float makeupDb = 0.0f;
    float maximumUpwardDb = 0.0f;
    float ratio = 1.0f;
    float thresholdDb = -30.0f;
    float activityFloorDb = -70.0f;
};

MacroSettings mapAmount(float amount) noexcept;
float gainToDecibels(float gain) noexcept;
float decibelsToGain(float decibels) noexcept;
float computeUpwardGainDb(float levelDb, const MacroSettings&) noexcept;

class RmsDetector
{
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept { meanSquare = 0.0f; }
    float process(float linkedMagnitude) noexcept;

private:
    double sampleRate = 44100.0;
    float attackCoefficient = 0.0f;
    float releaseCoefficient = 0.0f;
    float meanSquare = 0.0f;
};
}
