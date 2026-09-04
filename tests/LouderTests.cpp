#include "dsp/LouderDynamics.h"
#include "LouderEngine.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string_view>
#include <vector>

namespace
{
int failures = 0;
void expect(bool condition, std::string_view message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
void near(float actual, float expected, float tolerance, std::string_view message)
{
    expect(std::abs(actual - expected) <= tolerance, message);
}
}

int main()
{
    using namespace g3x::louder;
    using namespace g3x::louder::dsp;
    const auto neutral = mapAmount(0.0f);
    const auto maximum = mapAmount(10.0f);
    near(neutral.makeupDb, 0.0f, 0.001f, "zero amount has no makeup");
    near(neutral.maximumUpwardDb, 0.0f, 0.001f, "zero amount has no upward gain");
    near(maximum.makeupDb, 18.0f, 0.001f, "maximum nominal makeup is 18 dB");
    near(maximum.maximumUpwardDb, 12.0f, 0.001f, "upward gain is capped at 12 dB");

    auto previous = mapAmount(0.0f);
    for (int step = 1; step <= 1000; ++step)
    {
        const auto current = mapAmount(static_cast<float>(step) * 0.01f);
        expect(current.makeupDb >= previous.makeupDb, "makeup mapping is monotonic");
        expect(current.maximumUpwardDb >= previous.maximumUpwardDb,
               "upward mapping is monotonic");
        previous = current;
    }

    near(computeUpwardGainDb(-90.0f, maximum), 0.0f, 0.001f,
         "silence below activity floor is not raised");
    expect(computeUpwardGainDb(-55.0f, maximum) > 0.0f,
           "low-level material receives upward gain");
    expect(computeUpwardGainDb(0.0f, maximum) == 0.0f,
           "material above threshold receives no upward gain");
    expect(computeUpwardGainDb(-55.0f, maximum) <= 12.0f,
           "static curve respects upward cap");

    for (const auto sampleRate : { 44100.0, 48000.0, 96000.0, 192000.0 })
    {
        RmsDetector detector;
        detector.prepare(sampleRate);
        float envelope = 0.0f;
        for (int sample = 0; sample < static_cast<int>(sampleRate); ++sample)
            envelope = detector.process(0.5f);
        expect(std::isfinite(envelope) && envelope > 0.49f && envelope < 0.51f,
               "RMS detector converges at supported sample rate");
        const auto attacked = envelope;
        for (int sample = 0; sample < static_cast<int>(sampleRate * 0.01); ++sample)
            envelope = detector.process(0.0f);
        expect(envelope < attacked && envelope > 0.0f, "RMS release is gradual");
    }

    {
        LouderEngine engine;
        engine.prepare(48000.0, 2);
        const auto latency = engine.getLatencySamples();
        expect(latency == 48, "safe limiter uses one millisecond of lookahead");
        constexpr int sampleCount = 512;
        std::vector<float> left(sampleCount);
        std::vector<float> right(sampleCount);
        for (int sample = 0; sample < sampleCount; ++sample)
            left[static_cast<std::size_t>(sample)] = right[static_cast<std::size_t>(sample)]
                = 0.25f * std::sin(0.071f * static_cast<float>(sample));
        const auto reference = left;
        float* channels[] { left.data(), right.data() };
        engine.process(channels, 2, sampleCount);
        float maximumError = 0.0f;
        for (int sample = latency; sample < sampleCount; ++sample)
            maximumError = std::max(maximumError, std::abs(
                left[static_cast<std::size_t>(sample)]
                - reference[static_cast<std::size_t>(sample - latency)]));
        expect(maximumError < 1.0e-6f, "amount zero is identity after reported latency");
        expect(left == right, "stereo channels remain matched");

        EngineParameters parameters;
        parameters.amount = 10.0f;
        parameters.ceilingMode = CeilingMode::safe;
        engine.setParameters(parameters);
        std::vector<float> loud(8192, 2.0f);
        std::vector<float> quiet(8192, 0.2f);
        float* limitedChannels[] { loud.data(), quiet.data() };
        engine.process(limitedChannels, 2, static_cast<int>(loud.size()));
        const auto ceiling = decibelsToGain(-1.0f);
        expect(std::abs(loud.back()) <= ceiling + 1.0e-4f,
               "safe limiter respects its sample-peak ceiling");
        const auto linkedRatio = std::abs(quiet.back() / loud.back());
        expect(std::abs(linkedRatio - 0.1f) < 0.001f,
               "linked limiter preserves stereo balance");

        engine.reset();
        std::vector<float> silence(2048, 0.0f);
        silence[0] = std::numeric_limits<float>::quiet_NaN();
        float* mono[] { silence.data() };
        engine.process(mono, 1, static_cast<int>(silence.size()));
        expect(std::all_of(silence.begin(), silence.end(), [](float value)
                           { return std::isfinite(value) && value == 0.0f; }),
               "silence and non-finite input remain silent and finite");
    }

    if (failures == 0)
        std::cout << "All G3X One Louder M1 tests passed\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
