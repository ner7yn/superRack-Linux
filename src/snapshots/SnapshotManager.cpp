#include "SnapshotManager.h"
#include "../audio/AudioEngine.h"
#include <algorithm>

namespace sr {

SnapshotManager::SnapshotManager() : activeIndex_(-1) {}

SnapshotManager::~SnapshotManager() {}

juce::String SnapshotManager::addSnapshot(const juce::String& name) {
    SnapshotData snapshot;
    snapshot.name = name;
    snapshot.id = generateUUID();
    snapshots_.push_back(snapshot);
    setActiveIndex(snapshots_.size() - 1);
    return snapshot.id;
}

bool SnapshotManager::removeSnapshot(int index) {
    if (index < 0 || index >= (int)snapshots_.size()) return false;
    snapshots_.erase(snapshots_.begin() + index);
    if (activeIndex_ >= (int)snapshots_.size()) {
        activeIndex_ = snapshots_.size() - 1;
    }
    return true;
}

bool SnapshotManager::renameSnapshot(int index, const juce::String& newName) {
    if (index < 0 || index >= (int)snapshots_.size()) return false;
    snapshots_[index].name = newName;
    return true;
}

bool SnapshotManager::duplicateSnapshot(int index) {
    if (index < 0 || index >= (int)snapshots_.size()) return false;
    snapshots_.insert(snapshots_.begin() + index + 1, snapshots_[index]);
    setActiveIndex(index + 1);
    return true;
}

int SnapshotManager::getSnapshotCount() const {
    return snapshots_.size();
}

SnapshotData& SnapshotManager::getSnapshot(int index) {
    return snapshots_[juce::jlimit(0, snapshots_.size() - 1, index)];
}

const SnapshotData& SnapshotManager::getSnapshot(int index) const {
    return snapshots_[juce::jlimit(0, snapshots_.size() - 1, index)];
}

int SnapshotManager::getNextIndex() const {
    if (activeIndex_ < 0 || activeIndex_ >= (int)snapshots_.size() - 1) return -1;
    return activeIndex_ + 1;
}

int SnapshotManager::getPrevIndex() const {
    if (activeIndex_ <= 0) return -1;
    return activeIndex_ - 1;
}

void SnapshotManager::applySnapshot(int index, bool recallSafe[MAX_RACKS], AudioEngine& engine) {
    if (index < 0 || index >= (int)snapshots_.size()) return;

    // Initialize recallSafe array if needed
    if (!recallSafe) {
        bool safeArray[MAX_RACKS] = {};
        recallSafe = safeArray;
    }

    auto& snapshot = snapshots_[index];
    engine.applySnapshot(snapshot, recallSafe);
    activeIndex_ = index;
}

void SnapshotManager::clearRecallSafe() {
    // Reset all recall-safe flags
    for (auto& snap : snapshots_) {
        std::fill(std::begin(snap.recallSafe), std::end(snap.recallSafe), false);
    }
}

bool SnapshotManager::saveSnapshots(const juce::File& file) const {
    // Implementation would serialize snapshots to JSON
    JUCE_UNUSED(file);
    return false;
}

bool SnapshotManager::loadSnapshots(const juce::File& file) {
    // Implementation would deserialize snapshots from JSON
    JUCE_UNUSED(file);
    return false;
}

} // namespace sr