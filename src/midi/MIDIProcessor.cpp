#include "MIDIProcessor.h"
#include <algorithm>

namespace sr {

MIDIProcessor::MIDIProcessor() : callback_(nullptr) {}

MIDIProcessor::~MIDIProcessor() {
    closeDevice();
}

void MIDIProcessor::openDevice(const juce::String& deviceName) {
    if (input_) {
        input_->close();
        delete input_;
        input_ = nullptr;
    }

    inputDevice_ = deviceName;

    // Get input devices
    juce::StringArray devices = juce::MidiInput::getAvailableDevices();
    for (int i = 0; i < devices.size(); ++i) {
        if (devices[i].contains(deviceName)) {
            input_ = juce::MidiInput::openDevice(i, this);
            break;
        }
    }

    // If not found, try first device
    if (!input_ && devices.size() > 0) {
        input_ = juce::MidiInput::openDevice(0, this);
        inputDevice_ = devices[0];
    }
}

void MIDIProcessor::closeDevice() {
    if (input_) {
        input_->close();
        delete input_;
        input_ = nullptr;
    }
}

const juce::StringArray MIDIProcessor::getAvailableDevices() const {
    return juce::MidiInput::getAvailableDevices();
}

void MIDIProcessor::addMapping(const MIDIMapping& mapping) {
    mappings_.push_back(mapping);
}

void MIDIProcessor::removeMapping(int index) {
    if (index < 0 || index >= (int)mappings_.size()) return;
    mappings_.erase(mappings_.begin() + index);
}

const std::vector<MIDIMapping>& MIDIProcessor::getMappings() const {
    return mappings_;
}

void MIDIProcessor::clearMappings() {
    mappings_.clear();
}

void MIDIProcessor::processMessage(const juce::MidiMessage& message) {
    if (!message.isControllerChange() &&
        !message.isProgramChange() &&
        !message.isNoteOn() &&
        !message.isNoteOff()) return;

    processData(message.getRawData(), message.getRawDataSize());
}

void MIDIProcessor::processData(const uint8_t* data, int numBytes) {
    if (numBytes < 2) return;

    uint8_t status = data[0] & 0xF0;
    uint8_t channel = data[0] & 0x0F;
    uint8_t noteOrController = data[1];
    uint8_t value = numBytes > 2 ? data[2] : 0;

    // Apply channel filter if set
    if (inputChannel_ >= 0 && channel != inputChannel_) return;

    // Check mappings
    for (const auto& mapping : mappings_) {
        bool match = false;

        if (status == 0xC0 && mapping.action == MIDIMapping::Action::SetSnapshot) {
            // Program Change
            if (mapping.programNumber == noteOrController || mapping.programNumber < 0) {
                match = true;
            }
        } else if (status == 0xB0 && mapping.action != MIDIMapping::Action::None) {
            // Control Change
            if (mapping.controllerNumber == noteOrController) {
                match = true;
            }
        }

        if (match && callback_) {
            callback_(mapping);
            return;
        }
    }
}

void MIDIProcessor::startLearn() {
    learningMode_ = true;
    learnedMapping_ = MIDIMapping{};
}

void MIDIProcessor::stopLearn() {
    learningMode_ = false;
}

void MIDIProcessor::setInputChannel(int channel) {
    inputChannel_ = channel;
}

void MIDIProcessor::setInputDevice(const juce::String& device) {
    if (device != inputDevice_) {
        openDevice(device);
    }
}

} // namespace sr