#include "SessionManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>

namespace sr {
namespace json = nlohmann;

SessionManager::SessionManager() {
    newSession();
}

SessionManager::~SessionManager() {}

bool SessionManager::loadSession(const juce::File& file) {
    if (!file.existsAsFile()) return false;

    std::ifstream stream(file.getFullPathName().toStdString());
    if (!stream) return false;

    try {
        json::json_t data;
        stream >> data;

        if (!validateSession(data)) return false;

        // Save current to undo
        pushUndo();

        // Parse session
        session_.name = data.value("session", json::json_t()).value("name", juce::String("Untitled"));
        session_.schemaVersion = data.value("schemaVersion", 1);
        session_.sampleRate = data.value("session", json::json_t()).value("sampleRate", 48000);
        session_.bufferSize = data.value("session", json::json_t()).value("bufferSize", 64);

        // Parse racks
        session_.racks.clear();
        auto racksJson = data.value("racks", json::json_t::array_t());
        for (const auto& rack : racksJson) {
            RackState state;
            state.id = rack.value("id", 0);
            state.name = rack.value("name", juce::String("Rack " + juce::String(state.id)));
            state.inputGain = rack.value("inputGain", 1.0f);
            state.outputGain = rack.value("outputGain", 1.0f);
            state.muted = rack.value("muted", false);
            state.bypassed = rack.value("bypassed", false);
            state.mode = rack.value("mode", 0) == 1 ? RackMode::Stereo : RackMode::Mono;
            session_.racks.push_back(state);
        }

        // Parse routing
        session_.routing.clear();
        auto routingJson = data.value("routing", json::json_t::array_t());
        for (const auto& r : routingJson) {
            RoutingEntry entry;
            entry.physicalInput = r.value("physicalInput", 0);
            entry.programChannel = r.value("programChannel", 0);
            entry.rackId = r.value("rackId", 0);
            entry.physicalOutput = r.value("physicalOutput", 0);
            entry.mode = r.value("mode", 0) == 1 ? RackMode::Stereo : RackMode::Mono;
            session_.routing.push_back(entry);
        }

        lastSavePath_ = file.getFullPathName();
        return true;
    } catch (...) {
        return false;
    }
}

bool SessionManager::saveSession(const juce::File& file) {
    try {
        // Create backup of existing file
        if (file.existsAsFile()) {
            auto backup = file.getParentDirectory().getChildFile(file.getFileName() + ".backup");
            file.copyFileTo(backup);
        }

        // Create temp file
        auto tempFile = file.getParentDirectory().getChildFile(file.getFileName() + ".tmp");

        json::json_t root;
        root["schemaVersion"] = session_.schemaVersion;

        json::json_t sessionObj;
        sessionObj["name"] = session_.name.toStdString();
        sessionObj["sampleRate"] = (int)session_.sampleRate;
        sessionObj["bufferSize"] = session_.bufferSize;
        root["session"] = sessionObj;

        // Save racks
        json::json_t racksJson = json::json_t::array();
        for (const auto& rack : session_.racks) {
            json::json_t rackObj;
            rackObj["id"] = rack.id;
            rackObj["name"] = rack.name.toStdString();
            rackObj["mode"] = rack.mode == RackMode::Stereo ? 1 : 0;
            rackObj["inputGain"] = rack.inputGain;
            rackObj["outputGain"] = rack.outputGain;
            rackObj["muted"] = rack.muted;
            rackObj["bypassed"] = rack.bypassed;
            rackObj["groupId"] = rack.groupId;
            json::json_t pluginURIs = json::json_t::array();
            for (const auto& uri : rack.pluginURIs) {
                pluginURIs.push_back(uri.toStdString());
            }
            rackObj["plugins"] = pluginURIs;
            racksJson.push_back(rackObj);
        }
        root["racks"] = racksJson;

        // Save routing
        json::json_t routingJson = json::json_t::array();
        for (const auto& entry : session_.routing) {
            json::json_t entryObj;
            entryObj["physicalInput"] = entry.physicalInput;
            entryObj["programChannel"] = entry.programChannel;
            entryObj["rackId"] = entry.rackId;
            entryObj["physicalOutput"] = entry.physicalOutput;
            entryObj["mode"] = entry.mode == RackMode::Stereo ? 1 : 0;
            routingJson.push_back(entryObj);
        }
        root["routing"] = routingJson;

        // Write to temp file
        std::ofstream stream(tempFile.getFullPathName().toStdString());
        stream << root.dump(2);
        stream.close();

        // Atomic rename
        tempFile.replaceFileContents(file);
        lastSavePath_ = file.getFullPathName();
        return true;
    } catch (...) {
        return false;
    }
}

bool SessionManager::saveSessionBackup(const juce::File& file) {
    try {
        auto backup = file.getParentDirectory().getChildFile(file.getFileName() + ".backup");
        if (file.existsAsFile()) {
            file.copyFileTo(backup);
        }
        return true;
    } catch (...) {
        return false;
    }
}

void SessionManager::newSession(const juce::String& name) {
    pushUndo();
    session_ = SessionData();
    session_.name = name;
    session_.schemaVersion = 1;
    session_.sampleRate = DEFAULT_SAMPLE_RATE;
    session_.bufferSize = DEFAULT_BUFFER_SIZE;
}

juce::String SessionManager::getLastSavePath() const {
    return lastSavePath_;
}

void SessionManager::setLastSavePath(const juce::String& path) {
    lastSavePath_ = path;
}

void SessionManager::undo() {
    if (undoStack_.empty()) return;
    redoStack_.push_back(session_);
    session_ = undoStack_.back();
    undoStack_.pop_back();
}

void SessionManager::redo() {
    if (redoStack_.empty()) return;
    undoStack_.push_back(session_);
    session_ = redoStack_.back();
    redoStack_.pop_back();
}

void SessionManager::pushUndo() {
    undoStack_.push_back(session_);
    if (undoStack_.size() > MAX_UNDO) {
        undoStack_.erase(undoStack_.begin());
    }
    redoStack_.clear();
}

bool SessionManager::validateSession(const juce::var& json) const {
    // Basic validation
    if (!json.isObject()) return false;
    auto obj = json.getDynamicObject();
    if (!obj) return false;

    auto version = obj->getProperty("schemaVersion").getIntValue();
    if (version <= 0) return false;

    return true;
}

int SessionManager::getSchemaVersion() const {
    return session_.schemaVersion;
}

} // namespace sr