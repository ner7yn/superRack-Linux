#include "RoutingMatrix.h"
#include "../racks/RackProcessor.h"
#include <algorithm>
#include <cstring>

namespace sr {

RoutingMatrix::RoutingMatrix() {
    std::memset(&config_, 0, sizeof(config_));
    configPtr_.store(&config_);
}

RoutingMatrix::~RoutingMatrix() {
    for (int i = 0; i < MAX_CHANNELS; ++i) {
        delete[] routeBufferIn_[i];
        delete[] routeBufferOut_[i];
    }
}

void RoutingMatrix::setRouting(const std::vector<RoutingEntry>& routing) {
    std::lock_guard<std::mutex> lock(configMutex_);
    std::memset(&config_, 0, sizeof(config_));

    for (const auto& entry : routing) {
        if (entry.programChannel >= 0 && entry.programChannel < MAX_CHANNELS) {
            config_.inputToChannel[entry.physicalInput] = entry.programChannel;
            config_.channelToRack[entry.programChannel] = entry.rackId;
            if (entry.mode == RackMode::Stereo) {
                config_.channelStereo[entry.programChannel] = true;
            }
        }
        if (entry.rackId >= 0 && entry.rackId < MAX_RACKS) {
            config_.rackToOutput[entry.rackId] = entry.physicalOutput;
            if (entry.mode == RackMode::Stereo) {
                config_.rackStereo[entry.rackId] = true;
            }
        }
    }

    // Count active channels
    config_.numActiveChannels = 0;
    config_.numActiveRacks = 0;
    for (int i = 0; i < MAX_CHANNELS; ++i) {
        if (config_.channelToRack[i] >= 0) {
            config_.activeChannels[config_.numActiveChannels++] = i;
        }
    }
    for (int i = 0; i < MAX_RACKS; ++i) {
        if (config_.rackToOutput[i] >= 0) {
            config_.activeRacks[config_.numActiveRacks++] = i;
        }
    }

    ensureRouteBuffer(512);
}

void RoutingMatrix::ensureRouteBuffer(int numSamples) {
    for (int i = 0; i < MAX_CHANNELS; ++i) {
        if (!routeBufferIn_[i]) {
            routeBufferIn_[i] = new float[numSamples];
        }
        if (!routeBufferOut_[i]) {
            routeBufferOut_[i] = new float[numSamples];
        }
    }
    routeBufferSize_ = numSamples;
}

void RoutingMatrix::setChannelToRack(int channel, int rackId, RackMode mode) {
    if (channel < 0 || channel >= MAX_CHANNELS) return;
    config_.channelToRack[channel] = rackId;
    if (mode == RackMode::Stereo) {
        config_.channelStereo[channel] = true;
    }
}

void RoutingMatrix::setRackToOutput(int rackId, int outputChannel, RackMode mode) {
    if (rackId < 0 || rackId >= MAX_RACKS) return;
    config_.rackToOutput[rackId] = outputChannel;
    if (mode == RackMode::Stereo) {
        config_.rackStereo[rackId] = true;
    }
}

void RoutingMatrix::setInputToChannel(int input, int channel) {
    if (input < 0 || input >= MAX_CHANNELS || channel < 0 || channel >= MAX_CHANNELS) return;
    config_.inputToChannel[input] = channel;
}

void RoutingMatrix::processBlock(const float* const* inputChannels,
                                  int numInputChannels,
                                  float* const* outputChannels,
                                  int numOutputChannels,
                                  int numSamples,
                                  RackProcessor* racks) {
    if (!racks) return;

    const Config* config = configPtr_.load(std::memory_order_acquire);
    if (!config) return;

    // Ensure buffers
    if (numSamples > routeBufferSize_) {
        ensureRouteBuffer(numSamples);
    }

    // Copy inputs to routing buffers
    for (int i = 0; i < numInputChannels && i < MAX_CHANNELS; ++i) {
        if (inputChannels[i]) {
            std::memcpy(routeBufferIn_[i], inputChannels[i], numSamples * sizeof(float));
        } else {
            std::memset(routeBufferIn_[i], 0, numSamples * sizeof(float));
        }
    }

    // Route from inputs to racks
    for (int i = 0; i < config->numActiveChannels; ++i) {
        int channel = config->activeChannels[i];
        int rackId = config->channelToRack[channel];
        if (rackId < 0 || rackId >= MAX_RACKS) continue;

        RackProcessor* rack = &racks[rackId];
        if (!rack) continue;

        if (config->channelStereo[channel]) {
            // Stereo route
            if (channel + 1 < numInputChannels && channel + 2 <= numOutputChannels) {
                float* outL = outputChannels[channel];
                float* outR = channel + 1 < numOutputChannels ? outputChannels[channel + 1] : nullptr;
                float* inL = routeBufferIn_[channel];
                float* inR = channel + 1 < numInputChannels ? routeBufferIn_[channel + 1] : nullptr;

                // Process left channel
                rack->processBlock(inL, outL, numSamples);
                if (inR && outR) {
                    rack->processBlock(inR, outR, numSamples);
                }
            }
        } else {
            // Mono route
            int outChan = config->rackToOutput[rackId];
            if (outChan >= 0 && outChan < numOutputChannels) {
                rack->processBlock(routeBufferIn_[channel], outputChannels[outChan], numSamples);
            }
        }
    }

    // Clear unused outputs
    for (int i = 0; i < numOutputChannels; ++i) {
        bool used = false;
        for (int j = 0; j < config->numActiveRacks; ++j) {
            int rackId = config->activeRacks[j];
            if (config->rackToOutput[rackId] == i) {
                used = true;
                break;
            }
        }
        if (!used && outputChannels[i]) {
            std::memset(outputChannels[i], 0, numSamples * sizeof(float));
        }
    }
}

void RoutingMatrix::reset() {
    std::memset(&config_, 0, sizeof(config_));
    configPtr_.store(&config_);
}

int RoutingMatrix::getRackForChannel(int channel) const {
    const Config* config = configPtr_.load(std::memory_order_acquire);
    if (!config || channel < 0 || channel >= MAX_CHANNELS) return -1;
    return config->channelToRack[channel];
}

int RoutingMatrix::getOutputForRack(int rackId) const {
    const Config* config = configPtr_.load(std::memory_order_acquire);
    if (!config || rackId < 0 || rackId >= MAX_RACKS) return -1;
    return config->rackToOutput[rackId];
}

int RoutingMatrix::getInputForChannel(int channel) const {
    const Config* config = configPtr_.load(std::memory_order_acquire);
    if (!config || channel < 0 || channel >= MAX_CHANNELS) return -1;
    for (int i = 0; i < MAX_CHANNELS; ++i) {
        if (config->inputToChannel[i] == channel) return i;
    }
    return -1;
}

bool RoutingMatrix::hasRoute(int rackId) const {
    const Config* config = configPtr_.load(std::memory_order_acquire);
    if (!config || rackId < 0 || rackId >= MAX_RACKS) return false;
    return config->rackToOutput[rackId] >= 0;
}

} // namespace sr