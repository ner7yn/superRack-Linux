#pragma once

#include <juce_core.h>
#include <juce_audio_processors.h>
#include <memory>
#include <atomic>
#include <vector>
#include "common/Types.h"

namespace sr {

class LV2PluginHost;

class RackProcessor {
public:
    RackProcessor(int id, LV2PluginHost& host);
    ~RackProcessor();

    // Configuration (message thread)
    void setName(const juce::String& name);
    void setMode(RackMode mode);
    void setInputGain(float gain);
    void setOutputGain(float gain);
    void setMuted(bool muted);
    void setBypassed(bool bypassed);
    void setGroupId(int groupId);
    void addPlugin(const juce::String& uri);
    void removePlugin(int index);
    void movePlugin(int from, int to);
    void bypassPlugin(int index, bool bypass);

    // Audio processing (real-time safe)
    void processBlock(const float* input, float* output, int numSamples);
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock);
    void releaseResources();
    void reset();

    // Accessors
    int getId() const { return id_; }
    const juce::String& getName() const { return name_; }
    RackMode getMode() const { return mode_; }
    float getInputGain() const { return inputGain_; }
    float getOutputGain() const { return outputGain_; }
    bool isMuted() const { return muted_; }
    bool isBypassed() const { return bypassed_; }
    int getGroupId() const { return groupId_; }
    int getNumPlugins() const { return static_cast<int>(plugins_.size()); }
    const juce::String& getPluginName(int index) const;

    // Metering
    float getInputPeak() const { return inputPeak_.load(); }
    float getOutputPeak() const { return outputPeak_.load(); }
    bool getInputClipped() const { return inputClipped_.load(); }
    bool getOutputClipped() const { return outputClipped_.load(); }

    // State
    void saveState(RackState& state) const;
    void restoreState(const RackState& state);

    // Plugin state
    juce::var getPluginState(int index) const;
    void setPluginState(int index, const juce::var& state);

    // Bypass state
    bool isPluginBypassed(int index) const;

private:
    struct PluginInstance {
        juce::AudioProcessor* processor = nullptr;
        juce::AudioProcessor* bypassedProcessor = nullptr;
        bool bypassed = false;
        bool active = false;
        juce::String uri;
        std::unique_ptr<juce::AudioProcessor> owned;
    };

    void applyConfig();

    int id_;
    LV2PluginHost& host_;
    juce::String name_;
    RackMode mode_ = RackMode::Mono;
    float inputGain_ = 1.0f;
    float outputGain_ = 1.0f;
    bool muted_ = false;
    bool bypassed_ = false;
    int groupId_ = -1;

    std::vector<PluginInstance> plugins_;

    // Pre-allocated buffers
    float* bufferA_ = nullptr;
    float* bufferB_ = nullptr;
    int bufferSize_ = 0;

    // Metering (atomic for GUI thread)
    std::atomic<float> inputPeak_{0.0f};
    std::atomic<float> outputPeak_{0.0f};
    std::atomic<bool> inputClipped_{false};
    std::atomic<bool> outputClipped_{false};

    // Config snapshot for audio thread
    struct ConfigSnapshot {
        float inputGain = 1.0f;
        float outputGain = 1.0f;
        bool muted = false;
        bool bypassed = false;
        bool pluginBypassed[MAX_PLUGINS_PER_RACK];
        int numPlugins = 0;
    };
    std::atomic<ConfigSnapshot*> configSnapshot_{nullptr};

    double currentSampleRate_ = DEFAULT_SAMPLE_RATE;
    int maxSamplesPerBlock_ = 0;
};

} // namespace sr