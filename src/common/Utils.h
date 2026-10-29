#pragma once

#include <juce_core.h>
#include <juce_data_structures.h>
#include <juce_events.h>
#include <atomic>
#include <functional>

namespace sr {
namespace utils {

inline juce::String generateUUID() {
    return juce::String::createRandomUUID();
}

inline juce::String timestampNow() {
    return juce::Time::getCurrentTime().toString(true, true, false, true);
}

// Thread-safe atomic float read/write
inline float atomicReadFloat(const std::atomic<float>& a) {
    return a.load(std::memory_order_acquire);
}

inline void atomicWriteFloat(std::atomic<float>& a, float v) {
    a.store(v, std::memory_order_release);
}

// Smoothed meter — peak decay with RMS
class MeterSmoother {
public:
    void reset() {
        peak = 0.0f;
        rms = 0.0f;
        sumSquares = 0.0f;
        count = 0;
    }

    void processBlock(const float* data, int numSamples) {
        float blockPeak = 0.0f;
        double sumSq = 0.0;
        for (int i = 0; i < numSamples; ++i) {
            const float s = std::abs(data[i]);
            if (s > blockPeak) blockPeak = s;
            sumSq += (double)s * (double)s;
        }
        if (blockPeak > peak) peak = blockPeak;
        else peak = peak * 0.999f + blockPeak * 0.001f;

        rms = (float)std::sqrt(sumSq / juce::jmax(1, numSamples));
        sumSquares += sumSq;
        count += numSamples;
    }

    float getPeak() const { return peak; }
    float getRMS() const { return rms; }

private:
    float peak = 0.0f;
    float rms = 0.0f;
    double sumSquares = 0.0;
    int count = 0;
};

// Gain conversion
inline float dbToGain(float db) {
    return std::pow(10.0f, db / 20.0f);
}

inline float gainToDb(float gain) {
    if (gain <= 0.0001f) return -96.0f;
    return 20.0f * std::log10(gain);
}

inline float decibelsToLinear(float db) { return dbToGain(db); }
inline float linearToDecibels(float gain) { return gainToDb(gain); }

} // namespace utils
} // namespace sr