#pragma once

#include <juce_core.h>
#include <juce_data_structures.h>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <functional>

namespace sr {

constexpr int MAX_CHANNELS = 32;
constexpr int MAX_RACKS = 32;
constexpr int MAX_PLUGINS_PER_RACK = 16;
constexpr int DEFAULT_SAMPLE_RATE = 48000;
constexpr int DEFAULT_BUFFER_SIZE = 64;
constexpr int MAX_BUFFER_SIZE = 512;

enum class RackMode { Mono, Stereo };

struct PluginDescriptor {
    juce::String uri;
    juce::String name;
    juce::String manufacturer;
    juce::String category;
    juce::String version;
    juce::String bundlePath;
    bool hasGUI = false;
    bool hasState = false;
    bool loaded = false;
    juce::String loadError;
};

struct RackState {
    int id = 0;
    juce::String name;
    RackMode mode = RackMode::Mono;
    float inputGain = 1.0f;
    float outputGain = 1.0f;
    bool muted = false;
    bool bypassed = false;
    int groupId = -1;
    std::vector<juce::String> pluginURIs;
    std::vector<juce::var> pluginStates;
};

struct RoutingEntry {
    int physicalInput = 0;
    int programChannel = 0;
    int rackId = 0;
    int physicalOutput = 0;
    RackMode mode = RackMode::Mono;
};

struct GroupState {
    int id = 0;
    juce::String name;
    juce::Colour colour;
    std::vector<int> rackIds;
    bool groupMute = false;
    bool groupBypass = false;
};

struct MeterData {
    std::atomic<float> inputPeak{0.0f};
    std::atomic<float> outputPeak{0.0f};
    std::atomic<float> inputRMS{0.0f};
    std::atomic<float> outputRMS{0.0f};
    std::atomic<bool> inputClipped{false};
    std::atomic<bool> outputClipped{false};

    void reset() {
        inputPeak.store(0.0f);
        outputPeak.store(0.0f);
        inputRMS.store(0.0f);
        outputRMS.store(0.0f);
        inputClipped.store(false);
        outputClipped.store(false);
    }
};

struct AudioStatus {
    std::atomic<bool> deviceConnected{false};
    std::atomic<double> sampleRate{0.0};
    std::atomic<int> bufferSize{0};
    std::atomic<int> xrunCount{0};
    std::atomic<float> cpuLoad{0.0f};
    juce::String deviceName;
    juce::String errorMessage;
};

struct MIDIMapping {
    enum class Action { None, Mute, Bypass, Gain, NextSnapshot, PrevSnapshot, SetSnapshot };
    Action action = Action::None;
    int channel = -1;
    int controllerNumber = -1;
    int programNumber = -1;
    int rackId = -1;
    float gainValue = 1.0f;
};

struct SnapshotData {
    juce::String name;
    juce::String id;
    std::vector<RackState> racks;
    std::vector<RoutingEntry> routing;
    std::vector<GroupState> groups;
    std::vector<std::pair<juce::String, juce::var>> pluginParams;
    bool recallSafe[MAX_RACKS]{false};
};

struct SessionData {
    juce::String name = "Live Session";
    int schemaVersion = 1;
    double sampleRate = DEFAULT_SAMPLE_RATE;
    int bufferSize = DEFAULT_BUFFER_SIZE;
    std::vector<RackState> racks;
    std::vector<RoutingEntry> routing;
    std::vector<GroupState> groups;
    std::vector<SnapshotData> snapshots;
    std::vector<MIDIMapping> midiMappings;
    int activeSnapshot = -1;
};

} // namespace sr