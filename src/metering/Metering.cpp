#include "Metering.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace sr {

Metering::Metering() {
    inputMeters_.resize(MAX_CHANNELS);
    outputMeters_.resize(MAX_CHANNELS);
}

Metering::~Metering() {}

void Metering::processBlock(const float* const* channels, int numChannels, int numSamples) {
    int numCh = juce::jmin(numChannels, numChannels_);

    for (int ch = 0; ch < numCh; ++ch) {
        const float* data = channels[ch];
        if (!data) continue;

        float blockPeak = 0.0f;
        double sumSq = 0.0;

        for (int i = 0; i < numSamples; ++i) {
            const float s = std::abs(data[i]);
            if (s > blockPeak) blockPeak = s;
            sumSq += (double)s * (double)s;
        }

        auto& meter = inputMeters_[ch];
        if (blockPeak > meter.peakHold) {
            meter.peakHold = blockPeak;
            meter.peak.store(blockPeak, std::memory_order_relaxed);
        } else {
            meter.peakHold = meter.peakHold * 0.95f + blockPeak * 0.05f;
            meter.peak.store(meter.peakHold, std::memory_order_relaxed);
        }

        float rms = (float)std::sqrt(sumSq / juce::jmax(1, numSamples));
        meter.rms.store(rms, std::memory_order_relaxed);

        bool clipped = blockPeak >= 1.0f;
        meter.clipped.store(clipped, std::memory_order_relaxed);
    }
}

float Metering::getInputPeak(int channel) const {
    if (channel < 0 || channel >= numChannels_) return 0.0f;
    return inputMeters_[channel].peak.load(std::memory_order_relaxed);
}

float Metering::getInputRMS(int channel) const {
    if (channel < 0 || channel >= numChannels_) return 0.0f;
    return inputMeters_[channel].rms.load(std::memory_order_relaxed);
}

bool Metering::getInputClipped(int channel) const {
    if (channel < 0 || channel >= numChannels_) return false;
    return inputMeters_[channel].clipped.load(std::memory_order_relaxed);
}

float Metering::getOutputPeak(int channel) const {
    if (channel < 0 || channel >= numChannels_) return 0.0f;
    return outputMeters_[channel].peak.load(std::memory_order_relaxed);
}

float Metering::getOutputRMS(int channel) const {
    if (channel < 0 || channel >= numChannels_) return 0.0f;
    return outputMeters_[channel].rms.load(std::memory_order_relaxed);
}

bool Metering::getOutputClipped(int channel) const {
    if (channel < 0 || channel >= numChannels_) return false;
    return outputMeters_[channel].clipped.load(std::memory_order_relaxed);
}

float Metering::getOverallInputPeak() const {
    float peak = 0.0f;
    for (int i = 0; i < numChannels_; ++i) {
        peak = juce::jmax(peak, getInputPeak(i));
    }
    return peak;
}

float Metering::getOverallOutputPeak() const {
    float peak = 0.0f;
    for (int i = 0; i < numChannels_; ++i) {
        peak = juce::jmax(peak, getOutputPeak(i));
    }
    return peak;
}

void Metering::reset() {
    for (auto& m : inputMeters_) {
        m.peak.store(0.0f);
        m.rms.store(0.0f);
        m.clipped.store(false);
        m.peakHold = 0.0f;
    }
    for (auto& m : outputMeters_) {
        m.peak.store(0.0f);
        m.rms.store(0.0f);
        m.clipped.store(false);
        m.peakHold = 0.0f;
    }
}

void Metering::setNumChannels(int numChannels) {
    numChannels_ = juce::jmax(1, juce::jmin(numChannels, MAX_CHANNELS));
    inputMeters_.resize(numChannels_);
    outputMeters_.resize(numChannels_);
}

} // namespace sr