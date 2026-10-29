#pragma once

#include <juce_core.h>
#include <juce_data_structures.h>
#include "common/Types.h"

namespace sr {

class SessionManager {
public:
    SessionManager();
    ~SessionManager();

    // Load/save
    bool loadSession(const juce::File& file);
    bool saveSession(const juce::File& file);
    bool saveSessionBackup(const juce::File& file);

    // Access current session
    SessionData& getSession() { return session_; }
    const SessionData& getSession() const { return session_; }

    // Commands
    void newSession(const juce::String& name = "New Session");
    juce::String getLastSavePath() const;
    void setLastSavePath(const juce::String& path);

    // Undo/redo
    void undo();
    void redo();
    bool canUndo() const { return !undoStack_.empty(); }
    bool canRedo() const { return !redoStack_.empty(); }

    // Validation
    bool validateSession(const juce::var& json) const;
    int getSchemaVersion() const;

private:
    void pushUndo();
    void applyUndo(bool fromRedo);

    SessionData session_;
    std::vector<SessionData> undoStack_;
    std::vector<SessionData> redoStack_;
    juce::String lastSavePath_;
    static constexpr int MAX_UNDO = 50;
};

} // namespace sr