#pragma once

#include <juce_core.h>
#include <juce_data_structures.h>
#include <memory>
#include <atomic>
#include "common/Types.h"

namespace sr {

class RackProcessor;

class RoutingMatrix {
public:
    RoutingMatrix();
    ~RoutingMatrix();

    // Configuration (call from GUI/message thread, not audio thread)
    void setRouting(const std::vector<RoutingEntry>& routing);
    std::vector<RoutingEntry> getRouting() const;

    void setChannelToRack(int channel, int rackId, RackMode mode = RackMode::Mono);
    void setRackToOutput(int rackId, int outputChannel, RackMode mode = RackMode::Mono);
    void setInputToChannel(int input, int channel);

    // Audio processing (real-time safe)
    void processBlock(const float* const* inputChannels,
                      int numInputChannels,
                      float* const* outputChannels,
                      int numOutputChannels,
                      int numSamples,
                      RackProcessor* racks);

    void reset();

    // Query
    int getRackForChannel(int channel) const;
    int getOutputForRack(int rackId) const;
    int getInputForChannel(int channel) const;
    bool hasRoute(int rackId) const;

    // Stereo support
    bool isChannelStereo(int channel) const { return channelStereo_[channel]; }
    bool isRackStereo(int rackId) const { return rackStereo_[rackId]; }

private:
    // Lock-free configuration update
    void updateConfig();

    struct Config {
        int inputToChannel[MAX_CHANNELS];
        int channelToRack[MAX_CHANNELS];
        int rackToOutput[MAX_RACKS];
        bool channelStereo[MAX_CHANNELS];
        bool rackStereo[MAX_RACKS];
        int activeChannels[MAX_CHANNELS];
        int numActiveChannels = 0;
        int activeRacks[MAX_RACKS];
        int numActiveRacks = 0;
    };

    Config config_;
    std::atomic<Config*> configPtr_{nullptr};
    std::mutex configMutex_;

    // Pre-allocated buffers for routing
    float* routeBufferIn_[MAX_CHANNELS];
    float* routeBufferOut_[MAX_CHANNELS];
    int routeBufferSize_ = 0;

    void ensureRouteBuffer(int numSamples);
};

} // namespace sr