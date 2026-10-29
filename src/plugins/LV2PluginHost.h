#pragma once

#include <juce_core.h>
#include <juce_audio_processors.h>
#include <juce_data_structures.h>
#include <memory>
#include <vector>
#include <atomic>
#include "common/Types.h"

namespace sr {

class LV2PluginHost {
public:
    LV2PluginHost();
    ~LV2PluginHost();

    // Scanning
    void rescanPlugins();
    std::vector<PluginDescriptor> getAvailablePlugins() const;
    std::vector<PluginDescriptor> findPlugins(const juce::String& query) const;

    // Instance management
    juce::AudioProcessor* createInstance(const juce::String& uri, double sampleRate, int blockSize);
    bool destroyInstance(juce::AudioProcessor* instance);

    // State
    juce::var savePluginState(juce::AudioProcessor* instance);
    bool restorePluginState(juce::AudioProcessor* instance, const juce::var& state);

    // GUI
    bool showPluginGUI(juce::AudioProcessor* instance, juce::Component* parent);

    // Plugin info
    bool getPluginInfo(const juce::String& uri, PluginDescriptor& info) const;
    bool isPluginAvailable(const juce::String& uri) const;

    // Error handling
    juce::String getLastError() const { return lastError_; }

    // Thread-safe access
    std::vector<juce::AudioProcessor*> getInstances() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    juce::String lastError_;
    std::vector<PluginDescriptor> pluginCache_;
    mutable std::mutex cacheMutex_;
    std::mutex instanceMutex_;
};

} // namespace sr