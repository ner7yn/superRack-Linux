#pragma once

#include <juce_core.h>
#include <juce_events.h>
#include <memory>
#include <atomic>
#include <vector>
#include "common/Types.h"

namespace sr {

class Metering {
public:
    Metering();
    ~Metering();

    // Real-time safe: called from audio thread
    void processBlock(const float* const* channels, int numChannels, int numSamples);

    // GUI thread access
    float getInputPeak(int channel) const;
    float getInputRMS(int channel) const;
    bool getInputClipped(int channel) const;
    float getOutputPeak(int channel) const;
    float getOutputRMS(int channel) const;
    bool getOutputClipped(int channel) const;
    float getOverallInputPeak() const;
    float getOverallOutputPeak() const;

    void reset();

    // Configuration
    void setNumChannels(int numChannels);
    int getNumChannels() const { return numChannels_; }

    // Callback for GUI updates
    std::function<void()> onMeterUpdate;

private:
    struct ChannelMeter {
        std::atomic<float> peak{0.0f};
        std::atomic<float> rms{0.0f};
        std::atomic<bool> clipped{false};
        float peakHold = 0.0f;
    };

    std::vector<ChannelMeter> inputMeters_;
    std::vector<ChannelMeter> outputMeters_;
    int numChannels_ = MAX_CHANNELS;
    mutable std::mutex guiMutex_;
    double currentSampleRate_ = DEFAULT_SAMPLE_RATE;
};

} // namespace sr