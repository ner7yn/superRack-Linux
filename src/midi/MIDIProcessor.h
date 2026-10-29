#pragma once

#include <juce_core.h>
#include <juce_events.h>
#include <vector>
#include "common/Types.h"

namespace sr {

class MIDIProcessor {
public:
    MIDIProcessor();
    ~MIDIProcessor();

    // Device management
    void openDevice(const juce::String& deviceName);
    void closeDevice();
    const juce::StringArray getAvailableDevices() const;

    // Mapping
    void addMapping(const MIDIMapping& mapping);
    void removeMapping(int index);
    const std::vector<MIDIMapping>& getMappings() const;
    void clearMappings();

    // Process incoming MIDI
    void processMessage(const juce::MidiMessage& message);
    void processData(const uint8_t* data, int numBytes);

    // Learn mode
    bool isLearning() const { return learningMode_; }
    void startLearn();
    void stopLearn();
    MIDIMapping getLearnedMapping() const { return learnedMapping_; }

    // Integration with engine
    void setCallback(std::function<void(MIDIMapping)> callback) { callback_ = callback; }

    // Channel settings
    void setInputChannel(int channel);
    void setInputDevice(const juce::String& device);

private:
    juce::MidiInput* input_ = nullptr;
    std::vector<MIDIMapping> mappings_;
    std::function<void(MIDIMapping)> callback_;
    bool learningMode_ = false;
    MIDIMapping learnedMapping_;
    int inputChannel_ = -1;
    juce::String inputDevice_;
};

} // namespace sr