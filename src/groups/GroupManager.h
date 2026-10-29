#pragma once

#include <juce_core.h>
#include <vector>
#include "common/Types.h"

namespace sr {

class GroupManager {
public:
    GroupManager();
    ~GroupManager();

    // Management
    int createGroup(const juce::String& name, juce::Colour colour);
    bool removeGroup(int groupId);
    bool renameGroup(int groupId, const juce::String& name);
    void setGroupColour(int groupId, juce::Colour colour);

    // Members
    bool addRackToGroup(int groupId, int rackId);
    bool removeRackFromGroup(int groupId, int rackId);
    bool isRackInGroup(int rackId) const;
    int getGroupForRack(int rackId) const;

    // Access
    const std::vector<GroupState>& getGroups() const { return groups_; }
    GroupState& getGroup(int id);
    const GroupState& getGroup(int id) const;
    int getGroupCount() const { return groups_.size(); }

    // Bulk operations
    void groupMute(int groupId, bool muted);
    void groupBypass(int groupId, bool bypassed);
    void groupMuteAll(bool muted);
    void groupBypassAll(bool bypassed);

private:
    std::vector<GroupState> groups_;
    std::map<int, int> rackToGroup_;
};

} // namespace sr