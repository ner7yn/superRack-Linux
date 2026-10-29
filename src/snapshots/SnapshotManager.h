#pragma once

#include <juce_core.h>
#include <juce_data_structures.h>
#include <vector>
#include <memory>
#include "common/Types.h"

namespace sr {

class SnapshotManager {
public:
    SnapshotManager();
    ~SnapshotManager();

    // Management
    juce::String addSnapshot(const juce::String& name);
    bool removeSnapshot(int index);
    bool renameSnapshot(int index, const juce::String& newName);
    bool duplicateSnapshot(int index);
    int getSnapshotCount() const;
    SnapshotData& getSnapshot(int index);
    const SnapshotData& getSnapshot(int index) const;
    int getActiveIndex() const { return activeIndex_; }
    void setActiveIndex(int index) { activeIndex_ = juce::jlimit(-1, getSnapshotCount() - 1, index); }

    // Navigation
    int getNextIndex() const;
    int getPrevIndex() const;

    // Apply/Load
    void applySnapshot(int index, bool recallSafe[MAX_RACKS], class AudioEngine& engine);
    void clearRecallSafe();

    // Persistence
    bool saveSnapshots(const juce::File& file) const;
    bool loadSnapshots(const juce::File& file);

private:
    std::vector<SnapshotData> snapshots_;
    int activeIndex_ = -1;
};

} // namespace sr