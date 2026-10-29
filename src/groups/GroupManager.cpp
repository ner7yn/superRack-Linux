#include "GroupManager.h"
#include <algorithm>

namespace sr {

GroupManager::GroupManager() {}

GroupManager::~GroupManager() {}

int GroupManager::createGroup(const juce::String& name, juce::Colour colour) {
    GroupState group;
    group.id = groups_.size();
    group.name = name;
    group.colour = colour;
    groups_.push_back(group);
    return group.id;
}

bool GroupManager::removeGroup(int groupId) {
    for (auto it = groups_.begin(); it != groups_.end(); ++it) {
        if (it->id == groupId) {
            groups_.erase(it);
            // Remove rack mappings
            for (auto it2 = rackToGroup_.begin(); it2 != rackToGroup_.end(); ) {
                if (it2->second == groupId) {
                    it2 = rackToGroup_.erase(it2);
                } else {
                    ++it2;
                }
            }
            // Reassign IDs
            for (size_t i = 0; i < groups_.size(); ++i) {
                groups_[i].id = i;
            }
            return true;
        }
    }
    return false;
}

bool GroupManager::renameGroup(int groupId, const juce::String& name) {
    for (auto& group : groups_) {
        if (group.id == groupId) {
            group.name = name;
            return true;
        }
    }
    return false;
}

void GroupManager::setGroupColour(int groupId, juce::Colour colour) {
    for (auto& group : groups_) {
        if (group.id == groupId) {
            group.colour = colour;
            break;
        }
    }
}

bool GroupManager::addRackToGroup(int groupId, int rackId) {
    for (auto& group : groups_) {
        if (group.id == groupId) {
            if (std::find(group.rackIds.begin(), group.rackIds.end(), rackId) == group.rackIds.end()) {
                group.rackIds.push_back(rackId);
            }
            rackToGroup_[rackId] = groupId;
            return true;
        }
    }
    return false;
}

bool GroupManager::removeRackFromGroup(int groupId, int rackId) {
    for (auto& group : groups_) {
        if (group.id == groupId) {
            group.rackIds.erase(
                std::remove(group.rackIds.begin(), group.rackIds.end(), rackId),
                group.rackIds.end());
            rackToGroup_.erase(rackId);
            return true;
        }
    }
    return false;
}

bool GroupManager::isRackInGroup(int rackId) const {
    return rackToGroup_.count(rackId) > 0;
}

int GroupManager::getGroupForRack(int rackId) const {
    auto it = rackToGroup_.find(rackId);
    return it != rackToGroup_.end() ? it->second : -1;
}

GroupState& GroupManager::getGroup(int id) {
    for (auto& group : groups_) {
        if (group.id == id) return group;
    }
    return groups_.front();
}

const GroupState& GroupManager::getGroup(int id) const {
    for (const auto& group : groups_) {
        if (group.id == id) return group;
    }
    return groups_.front();
}

void GroupManager::groupMute(int groupId, bool muted) {
    for (auto& group : groups_) {
        if (group.id == groupId) {
            group.groupMute = muted;
            for (int rackId : group.rackIds) {
                // Signal to AudioEngine to mute rack
                JUCE_UNUSED(rackId);
            }
            break;
        }
    }
}

void GroupManager::groupBypass(int groupId, bool bypassed) {
    for (auto& group : groups_) {
        if (group.id == groupId) {
            group.groupBypass = bypassed;
            for (int rackId : group.rackIds) {
                JUCE_UNUSED(rackId);
            }
            break;
        }
    }
}

void GroupManager::groupMuteAll(bool muted) {
    for (auto& group : groups_) {
        group.groupMute = muted;
    }
}

void GroupManager::groupBypassAll(bool bypassed) {
    for (auto& group : groups_) {
        group.groupBypass = bypassed;
    }
}

} // namespace sr