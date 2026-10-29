#include "LV2PluginHost.h"
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/extensions/atom/atom.h>
#include <lv2/lv2plug.in/ns/extensions/urid/urid.h>
#include <lilv/lilv.h>
#include <mutex>
#include <map>

namespace sr {

class LV2PluginHost::Impl {
public:
    LilvWorld* world = nullptr;
    LilvPlugins* plugins = nullptr;
    LilvInstances* instances = nullptr;

    struct PluginInfo {
        LilvPlugin* plugin = nullptr;
        juce::String uri;
        PluginDescriptor descriptor;
    };

    std::map<juce::String, PluginInfo> pluginMap;
    std::map<void*, juce::AudioProcessor*> instanceMap;
};

LV2PluginHost::LV2PluginHost() : impl_(std::make_unique<Impl>()) {
    impl_->world = lilv_world_new();
    impl_->plugins = lilv_plugins_new(impl_->world, nullptr, nullptr);
}

LV2PluginHost::~LV2PluginHost() {
    // Clean up instances
    for (auto& [uri, processor] : impl_->instanceMap) {
        if (processor) {
            delete processor;
        }
    }
    impl_->instanceMap.clear();

    if (impl_->plugins) {
        lilv_plugins_free(impl_->plugins);
    }
    if (impl_->world) {
        lilv_world_free(impl_->world);
    }
}

void LV2PluginHost::rescanPlugins() {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    pluginCache_.clear();
    impl_->pluginMap.clear();

    lilv_nodes_free(lilv_plugins_get_all(impl_->plugins));

    LilvIter* iter = lilv_plugins_begin(impl_->plugins);
    const LilvPlugin* plugin = nullptr;

    while ((plugin = lilv_plugins_next(impl_->plugins, iter)) != nullptr) {
        const LilvURI* uri = lilv_plugin_get_urisuite(plugin);
        if (!uri) continue;

        PluginDescriptor desc;
        desc.uri = lilv_node_get_string(uri);

        const LilvNode* nameNode = lilv_plugin_get_name(plugin);
        if (nameNode) desc.name = lilv_node_get_string(nameNode);

        const LilvNode* manuNode = lilv_plugin_get_plugin_name(plugin, lilv_world_get_uri(impl_->world, LV2_CORE__plugin));
        // Try manufacturer
        const LilvNodes* manuNodes = lilv_plugin_get(plugin,
            lilv_world_get_uri(impl_->world, LV2_CORE__maker));
        if (manuNodes) {
            LilvNode* manu = lilv_nodes_get(manuNodes, 0);
            if (manu) desc.manufacturer = lilv_node_get_string(manu);
        }

        desc.loaded = true;
        pluginCache_.push_back(desc);
    }
    lilv_iter_free(iter);
}

std::vector<PluginDescriptor> LV2PluginHost::getAvailablePlugins() const {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    return pluginCache_;
}

std::vector<PluginDescriptor> LV2PluginHost::findPlugins(const juce::String& query) const {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    std::vector<PluginDescriptor> result;

    for (const auto& desc : pluginCache_) {
        if (desc.name.contains(query, false) ||
            desc.uri.contains(query, false) ||
            desc.manufacturer.contains(query, false)) {
            result.push_back(desc);
        }
    }
    return result;
}

juce::AudioProcessor* LV2PluginHost::createInstance(const juce::String& uri, double sampleRate, int blockSize) {
    std::lock_guard<std::mutex> lock(instanceMutex_);

    // Create a minimal plugin instance using JUCE's built-in plugin format
    // (LV2 support in JUCE is limited, we use a placeholder approach)
    return nullptr;
}

bool LV2PluginHost::destroyInstance(juce::AudioProcessor* instance) {
    if (instance) {
        delete instance;
        return true;
    }
    return false;
}

juce::var LV2PluginHost::savePluginState(juce::AudioProcessor* instance) {
    if (!instance) return juce::var();
    return juce::var();
}

bool LV2PluginHost::restorePluginState(juce::AudioProcessor* instance, const juce::var& state) {
    JUCE_UNUSED(instance);
    JUCE_UNUSED(state);
    return false;
}

bool LV2PluginHost::showPluginGUI(juce::AudioProcessor* instance, juce::Component* parent) {
    JUCE_UNUSED(instance);
    JUCE_UNUSED(parent);
    return false;
}

bool LV2PluginHost::getPluginInfo(const juce::String& uri, PluginDescriptor& info) const {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    for (const auto& desc : pluginCache_) {
        if (desc.uri == uri) {
            info = desc;
            return true;
        }
    }
    return false;
}

bool LV2PluginHost::isPluginAvailable(const juce::String& uri) const {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    for (const auto& desc : pluginCache_) {
        if (desc.uri == uri) {
            return desc.loaded;
        }
    }
    return false;
}

std::vector<juce::AudioProcessor*> LV2PluginHost::getInstances() const {
    std::lock_guard<std::mutex> lock(instanceMutex_);
    std::vector<juce::AudioProcessor*> result;
    for (const auto& [uri, proc] : impl_->instanceMap) {
        if (proc) result.push_back(proc);
    }
    return result;
}

} // namespace sr