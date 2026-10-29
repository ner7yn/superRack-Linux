#include "RackProcessor.h"
#include "../plugins/LV2PluginHost.h"
#include <algorithm>
#include <cstring>

namespace sr {

RackProcessor::RackProcessor(int id, LV2PluginHost& host)
    : id_(id), host_(host) {
    // Allocate buffers
    bufferA_ = new float[512];
    bufferB_ = new float[512];
    bufferSize_ = 512;
}

RackProcessor::~RackProcessor() {
    delete[] bufferA_;
    delete[] bufferB_;
}

void RackProcessor::setName(const juce::String& name) { name_ = name; }

void RackProcessor::setMode(RackMode mode) { mode_ = mode; }

void RackProcessor::setInputGain(float gain) {
    inputGain_ = juce::jlimit(0.0f, 10.0f, gain);
}

void RackProcessor::setOutputGain(float gain) {
    outputGain_ = juce::jlimit(0.0f, 10.0f, gain);
}

void RackProcessor::setMuted(bool muted) { muted_ = muted; }

void RackProcessor::setBypassed(bool bypassed) { bypassed_ = bypassed; }

void RackProcessor::setGroupId(int groupId) { groupId_ = groupId; }

void RackProcessor::addPlugin(const juce::String& uri) {
    if ((int)plugins_.size() >= MAX_PLUGINS_PER_RACK) return;
    PluginInstance inst;
    inst.uri = uri;
    inst.active = false;
    plugins_.push_back(inst);
}

void RackProcessor::removePlugin(int index) {
    if (index < 0 || index >= (int)plugins_.size()) return;
    plugins_.erase(plugins_.begin() + index);
}

void RackProcessor::movePlugin(int from, int to) {
    if (from < 0 || from >= (int)plugins_.size()) return;
    if (to < 0 || to >= (int)plugins_.size()) return;
    auto it = plugins_.begin() + from;
    auto temp = *it;
    plugins_.erase(it);
    plugins_.insert(plugins_.begin() + to, temp);
}

void RackProcessor::bypassPlugin(int index, bool bypass) {
    if (index < 0 || index >= (int)plugins_.size()) return;
    plugins_[index].bypassed = bypass;
}

void RackProcessor::processBlock(const float* input, float* output, int numSamples) {
    if (!input || !output || numSamples <= 0) return;

    // Handle buffer size
    if (numSamples > bufferSize_) {
        delete[] bufferA_;
        delete[] bufferB_;
        bufferSize_ = juce::jalign_power_of_two(numSamples);
        bufferA_ = new float[bufferSize_];
        bufferB_ = new float[bufferSize_];
    }

    // Copy input to buffer
    std::memcpy(bufferA_, input, numSamples * sizeof(float));

    // Apply input gain
    for (int i = 0; i < numSamples; ++i) {
        bufferA_[i] *= inputGain_;
    }

    // Track input peak
    float inPeak = 0.0f;
    for (int i = 0; i < numSamples; ++i) {
        inPeak = juce::jmax(inPeak, std::abs(bufferA_[i]));
    }
    inputPeak_.store(inPeak, std::memory_order_relaxed);
    inputClipped_.store(inPeak >= 1.0f, std::memory_order_relaxed);

    // Process through plugin chain
    float* current = bufferA_;
    float* next = bufferB_;

    for (int i = 0; i < (int)plugins_.size(); ++i) {
        auto& plugin = plugins_[i];
        if (!plugin.active || plugin.bypassed || !plugin.processor) {
            // Swap buffers
            std::swap(current, next);
            continue;
        }

        // Process with plugin
        // Note: In real implementation, this would use JUCE's audio callback
        // For MVP, we apply simple gain staging as placeholder
        for (int j = 0; j < numSamples; ++j) {
            next[j] = current[j]; // Placeholder
        }
        std::swap(current, next);
    }

    // Apply output gain
    for (int i = 0; i < numSamples; ++i) {
        output[i] = current[i] * outputGain_;
    }

    // Handle mute
    if (muted_) {
        std::memset(output, 0, numSamples * sizeof(float));
    }

    // Handle bypass
    if (bypassed_) {
        std::memcpy(output, input, numSamples * sizeof(float));
    }

    // Track output peak
    float outPeak = 0.0f;
    for (int i = 0; i < numSamples; ++i) {
        outPeak = juce::jmax(outPeak, std::abs(output[i]));
    }
    outputPeak_.store(outPeak, std::memory_order_relaxed);
    outputClipped_.store(outPeak >= 1.0f, std::memory_order_relaxed);
}

void RackProcessor::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate_ = sampleRate;
    maxSamplesPerBlock_ = maximumExpectedSamplesPerBlock;

    // Prepare all plugins
    for (auto& plugin : plugins_) {
        if (plugin.active && plugin.processor) {
            plugin.processor->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
        }
    }
}

void RackProcessor::releaseResources() {
    for (auto& plugin : plugins_) {
        if (plugin.active && plugin.processor) {
            plugin.processor->releaseResources();
        }
    }
}

void RackProcessor::reset() {
    inputPeak_.store(0.0f);
    outputPeak_.store(0.0f);
    inputClipped_.store(false);
    outputClipped_.store(false);

    for (auto& plugin : plugins_) {
        if (plugin.active && plugin.processor) {
            plugin.processor->reset();
        }
    }
}

const juce::String& RackProcessor::getPluginName(int index) const {
    static juce::String empty;
    if (index < 0 || index >= (int)plugins_.size()) return empty;
    return plugins_[index].uri; // URI as name for now
}

void RackProcessor::saveState(RackState& state) const {
    state.id = id_;
    state.name = name_;
    state.mode = mode_;
    state.inputGain = inputGain_;
    state.outputGain = outputGain_;
    state.muted = muted_;
    state.bypassed = bypassed_;
    state.groupId = groupId_;
    state.pluginURIs.clear();
    state.pluginStates.clear();

    for (const auto& plugin : plugins_) {
        state.pluginURIs.push_back(plugin.uri);
        // Save plugin state
        juce::var stateObj;
        if (plugin.active && plugin.processor) {
            // Placeholder - real implementation would use plugin state
        }
        state.pluginStates.push_back(stateObj);
    }
}

void RackProcessor::restoreState(const RackState& state) {
    name_ = state.name;
    mode_ = state.mode;
    inputGain_ = state.inputGain;
    outputGain_ = state.outputGain;
    muted_ = state.muted;
    bypassed_ = state.bypassed;
    groupId_ = state.groupId;

    // Restore plugins
    plugins_.clear();
    for (size_t i = 0; i < state.pluginURIs.size(); ++i) {
        PluginInstance inst;
        inst.uri = state.pluginURIs[i];
        inst.active = true;
        plugins_.push_back(inst);
    }
}

bool RackProcessor::isPluginBypassed(int index) const {
    if (index < 0 || index >= (int)plugins_.size()) return false;
    return plugins_[index].bypassed;
}

} // namespace sr