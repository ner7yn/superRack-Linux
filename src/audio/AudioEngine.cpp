#include "AudioEngine.h"
#include "../routing/RoutingMatrix.h"
#include "../racks/RackProcessor.h"
#include "../plugins/LV2PluginHost.h"
#include "../metering/Metering.h"
#include <algorithm>
#include <cstring>
#include <chrono>

namespace sr {

AudioEngine::AudioEngine()
    : deviceManager_([this](const juce::StringArray& devices) {
          // Callback when devices change
      }) {
    // Initialize routing
    routing_ = std::make_unique<RoutingMatrix>();

    // Initialize racks
    for (int i = 0; i < MAX_RACKS; ++i) {
        racks_[i] = std::make_unique<RackProcessor>(i, *pluginHost_);
    }

    // Initialize plugin host
    pluginHost_ = std::make_unique<LV2PluginHost>();
    pluginHost_->rescanPlugins();

    // Initialize metering
    metering_ = std::make_unique<Metering>();
    metering_->setNumChannels(MAX_CHANNELS);
}

AudioEngine::~AudioEngine() {}

const juce::String AudioEngine::getName() const {
    return "SuperRack Linux";
}

bool AudioEngine::acceptsMidi() const { return true; }
bool AudioEngine::producesMidi() const { return false; }
double AudioEngine::getTailLengthSeconds() const { return 0.0; }
int AudioEngine::getNumPrograms() { return 1; }
void AudioEngine::getCurrentProgramName(juce::String& name) { name = "Program 1"; }
void AudioEngine::setCurrentProgramName(const juce::String& name) { JUCE_UNUSED(name); }
bool AudioEngine::hasEditor() const { return true; }
bool AudioEngine::isBypassed() const { return false; }
void AudioEngine::setBypassed(bool shouldBypass) { JUCE_UNUSED(shouldBypass); }
void AudioEngine::programChanged(int index, const juce::String& newValue) { JUCE_UNUSED(index); JUCE_UNUSED(newValue); }
void AudioEngine::parameterChanged(const juce::String& parameterID, float newValue) { JUCE_UNUSED(parameterID); JUCE_UNUSED(newValue); }
void AudioEngine::suspend(bool shouldSuspend) { JUCE_UNUSED(shouldSuspend); }
void AudioEngine::reset() {}

juce::AudioProcessorEditor* AudioEngine::createEditor() {
    return nullptr; // Will be created by App component
}

bool AudioEngine::openAudioDevice(const juce::String& deviceName, double sampleRate, int bufferSize) {
    auto& manager = deviceManager_;

    // Get available devices
    juce::StringArray devices = manager.getAvailableDevices();
    if (devices.isEmpty()) {
        status_.deviceConnected.store(false);
        status_.errorMessage = "No audio devices found";
        return false;
    }

    // Find target device or use first available
    int deviceId = -1;
    for (int i = 0; i < devices.size(); ++i) {
        if (devices[i].contains(deviceName)) {
            deviceId = i;
            break;
        }
    }
    if (deviceId < 0 && devices.size() > 0) {
        deviceId = 0; // Use first device
    }

    if (deviceId < 0) {
        status_.deviceConnected.store(false);
        status_.errorMessage = "Device not found: " + deviceName;
        return false;
    }

    // Setup device with X32-like settings
    manager.setCurrentAudioDeviceIndex(deviceId);
    manager.setAudioDeviceSetup({
        .inputChannels = MAX_CHANNELS,
        .outputChannels = MAX_CHANNELS,
        .inputBufferSizes = {bufferSize},
        .outputBufferSizes = {bufferSize},
    });

    currentSampleRate_ = sampleRate;
    bufferSize_ = bufferSize;

    status_.deviceConnected.store(true);
    status_.sampleRate.store(sampleRate);
    status_.bufferSize.store(bufferSize);
    status_.deviceName = deviceName;

    // Initialize racks
    for (int i = 0; i < MAX_RACKS; ++i) {
        racks_[i]->prepareToPlay(sampleRate, bufferSize);
    }

    return true;
}

void AudioEngine::closeAudioDevice() {
    deviceManager_.clearAudioDevices();
    status_.deviceConnected.store(false);
}

const juce::StringArray AudioEngine::getAvailableDeviceNames() const {
    return deviceManager_.getAvailableDevices();
}

void AudioEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    const int numSamples = buffer.getNumSamples();
    const int numInputChannels = buffer.getNumChannels();
    const int numOutputChannels = buffer.getNumChannels();

    // Check for xruns
    if (numInputChannels != lastNumInputChannels_.load()) {
        status_.xrunCount.fetch_add(1, std::memory_order_relaxed);
        JUCE_LOG("XRUN detected: input channels changed from " +
                 juce::String(lastNumInputChannels_.load()) + " to " + juce::String(numInputChannels));
    }
    lastNumInputChannels_.store(numInputChannels);

    // Process MIDI
    processMidi(midiMessages);

    // Process audio
    processAudioBlock(buffer.getArrayOfWritePointers(),
                      buffer.getArrayOfWritePointers(),
                      numInputChannels,
                      numOutputChannels,
                      numSamples);
}

void AudioEngine::processAudioBlock(const float* const* inputChannels,
                                     const float* const* outputChannels,
                                     int numInputChannels,
                                     int numOutputChannels,
                                     int numSamples) {
    if (!status_.deviceConnected.load()) return;

    // Route audio through matrix
    routing_->processBlock(inputChannels, numInputChannels,
                           outputChannels, numOutputChannels,
                           numSamples,
                           &racks_[0]);

    // Update meters
    metering_->processBlock(inputChannels, numInputChannels, numSamples);
}

void AudioEngine::processMidi(const juce::MidiBuffer& midiMessages) {
    for (const auto& event : midiMessages) {
        const uint8_t* data = event.getRawData();
        int numBytes = event.getRawDataSize();
        processMidiInput(data, numBytes);
    }
}

void AudioEngine::applySnapshot(const SnapshotData& snapshot, bool recallSafe[MAX_RACKS]) {
    // Apply rack states
    for (int i = 0; i < MAX_RACKS && i < (int)snapshot.racks.size(); ++i) {
        if (recallSafe[i]) continue; // Skip recall-safe racks
        racks_[i]->restoreState(snapshot.racks[i]);
    }

    // Apply routing
    routing_->setRouting(snapshot.routing);

    // Reset meters
    metering_->reset();
}

void AudioEngine::saveSnapshot(SnapshotData& snapshot) const {
    snapshot.racks.clear();
    snapshot.routing = routing_->getRouting();

    for (int i = 0; i < MAX_RACKS; ++i) {
        RackState state;
        racks_[i]->saveState(state);
        snapshot.racks.push_back(state);
    }
}

void AudioEngine::processMidiInput(const uint8_t* data, int numBytes) {
    // Minimal MIDI processing - CC and PC
    if (numBytes < 2) return;

    uint8_t status = data[0] & 0xF0;
    uint8_t channel = data[0] & 0x0F;

    if (status == 0xB0) { // Control Change
        // CC processing
    } else if (status == 0xC0) { // Program Change
        // PC processing
    }
}

void AudioEngine::setBufferSize(int samples) {
    bufferSize_ = samples;
    for (int i = 0; i < MAX_RACKS; ++i) {
        racks_[i]->prepareToPlay(currentSampleRate_, samples);
    }
}

void AudioEngine::setRackMute(int rackId, bool muted) {
    if (rackId < 0 || rackId >= MAX_RACKS) return;
    racks_[rackId]->setMuted(muted);
}

void AudioEngine::setRackBypass(int rackId, bool bypassed) {
    if (rackId < 0 || rackId >= MAX_RACKS) return;
    racks_[rackId]->setBypassed(bypassed);
}

void AudioEngine::setRackInputGain(int rackId, float gain) {
    if (rackId < 0 || rackId >= MAX_RACKS) return;
    racks_[rackId]->setInputGain(gain);
}

void AudioEngine::setRackOutputGain(int rackId, float gain) {
    if (rackId < 0 || rackId >= MAX_RACKS) return;
    racks_[rackId]->setOutputGain(gain);
}

void AudioEngine::setRackName(int rackId, const juce::String& name) {
    if (rackId < 0 || rackId >= MAX_RACKS) return;
    racks_[rackId]->setName(name);
}

RackProcessor& AudioEngine::getRack(int id) {
    return *racks_[juce::jmax(0, juce::jmin(id, MAX_RACKS - 1))];
}

const RackProcessor& AudioEngine::getRack(int id) const {
    return *racks_[juce::jmax(0, juce::jmin(id, MAX_RACKS - 1))];
}

} // namespace sr