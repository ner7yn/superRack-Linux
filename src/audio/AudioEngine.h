#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <memory>
#include <atomic>
#include "common/Types.h"

namespace sr {

class RoutingMatrix;
class RackProcessor;
class LV2PluginHost;
class Metering;

class AudioEngine : public juce::AudioProcessor {
public:
    AudioEngine();
    ~AudioEngine() override;

    // AudioProcessor overrides
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    void getCurrentProgramName(juce::String& name) override;
    void setCurrentProgramName(const juce::String& name) override;
    bool hasEditor() const override;
    juce::AudioProcessorEditor* createEditor() override;
    bool isBypassed() const override;
    void setBypassed(bool shouldBypass) override;
    void programChanged(int index, const juce::String& newValue) override;
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void suspend(bool shouldSuspend) override;
    void reset() override;

    // Device management
    bool openAudioDevice(const juce::String& deviceName, double sampleRate, int bufferSize);
    void closeAudioDevice();
    const juce::StringArray getAvailableDeviceNames() const;
    const juce::AudioDeviceManager& getDeviceManager() const { return deviceManager_; }

    // Accessors
    RoutingMatrix& getRouting() { return *routing_; }
    const RoutingMatrix& getRouting() const { return *routing_; }
    RackProcessor& getRack(int id);
    const RackProcessor& getRack(int id) const;
    LV2PluginHost& getPluginHost() { return *pluginHost_; }
    Metering& getMetering() { return *metering_; }
    AudioStatus& getStatus() { return status_; }
    const AudioStatus& getStatus() const { return status_; }

    // Snapshot
    void applySnapshot(const SnapshotData& snapshot, bool recallSafe[MAX_RACKS]);
    void saveSnapshot(SnapshotData& snapshot) const;

    // MIDI
    void processMidiInput(const uint8_t* data, int numBytes);

    // Config
    void setBufferSize(int samples);
    int getBufferSize() const { return bufferSize_; }
    double getSampleRate() const { return currentSampleRate_; }

    // Thread-safe state access
    void setRackMute(int rackId, bool muted);
    void setRackBypass(int rackId, bool bypassed);
    void setRackInputGain(int rackId, float gain);
    void setRackOutputGain(int rackId, float gain);
    void setRackName(int rackId, const juce::String& name);

private:
    void processAudioBlock(const float* const* inputChannels,
                           const float* const* outputChannels,
                           int numInputChannels,
                           int numOutputChannels,
                           int numSamples);

    void processMidi(const juce::MidiBuffer& midiMessages);

    // Members
    juce::AudioDeviceManager deviceManager_;
    std::unique_ptr<juce::AudioDeviceManager> audioDeviceManager_;
    std::unique_ptr<juce::AudioPluginFormatManager> pluginFormatManager_;
    std::unique_ptr<juce::AudioProcessorPlayer> processorPlayer_;
    std::unique_ptr<juce::AudioSourcePlayer> audioSourcePlayer_;

    std::unique_ptr<RoutingMatrix> routing_;
    std::unique_ptr<RackProcessor> racks_[MAX_RACKS];
    std::unique_ptr<LV2PluginHost> pluginHost_;
    std::unique_ptr<Metering> metering_;

    AudioStatus status_;
    double currentSampleRate_ = DEFAULT_SAMPLE_RATE;
    int bufferSize_ = DEFAULT_BUFFER_SIZE;
    int numRacks_ = MAX_RACKS;

    // Thread-safe command queue for GUI -> audio thread
    struct PendingCommand {
        enum Type { Mute, Bypass, InputGain, OutputGain, Rename };
        Type type;
        int rackId;
        bool boolValue;
        float floatValue;
        juce::String stringValue;
    };
    std::vector<PendingCommand> pendingCommands_;
    std::mutex commandMutex_;
    std::atomic<bool> needsConfigUpdate_{false};

    // For xrun detection
    std::atomic<int> lastNumInputChannels_{0};
    std::atomic<int> lastNumOutputChannels_{0};

    JUCE_IMPLEMENT_IGNORE_OVERRIDE;
};

} // namespace sr