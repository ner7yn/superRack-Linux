#include <catch2/catch_test_macros.hpp>
#include "../src/snapshots/SnapshotManager.h"

using namespace sr;

TEST_CASE("Snapshot manager starts empty", "[Snapshot]") {
    SnapshotManager manager;
    REQUIRE(manager.getSnapshotCount() == 0);
    REQUIRE(manager.getActiveIndex() == -1);
}

TEST_CASE("Can add snapshot", "[Snapshot]") {
    SnapshotManager manager;
    auto id = manager.addSnapshot("Test Snapshot");

    REQUIRE(manager.getSnapshotCount() == 1);
    REQUIRE(manager.getActiveIndex() == 0);
    REQUIRE(id.isNotEmpty());
}

TEST_CASE("Can add multiple snapshots", "[Snapshot]") {
    SnapshotManager manager;
    manager.addSnapshot("First");
    manager.addSnapshot("Second");
    manager.addSnapshot("Third");

    REQUIRE(manager.getSnapshotCount() == 3);
    REQUIRE(manager.getSnapshot(0).name == "First");
    REQUIRE(manager.getSnapshot(1).name == "Second");
    REQUIRE(manager.getSnapshot(2).name == "Third");
}

TEST_CASE("Can navigate between snapshots", "[Snapshot]") {
    SnapshotManager manager;
    manager.addSnapshot("First");
    manager.addSnapshot("Second");
    manager.addSnapshot("Third");

    manager.setActiveIndex(1);
    REQUIRE(manager.getNextIndex() == 2);
    REQUIRE(manager.getPrevIndex() == 0);

    manager.setActiveIndex(0);
    REQUIRE(manager.getPrevIndex() == -1);

    manager.setActiveIndex(2);
    REQUIRE(manager.getNextIndex() == -1);
}

TEST_CASE("Can rename snapshot", "[Snapshot]") {
    SnapshotManager manager;
    manager.addSnapshot("Original");
    manager.renameSnapshot(0, "Renamed");

    REQUIRE(manager.getSnapshot(0).name == "Renamed");
}

TEST_CASE("Can duplicate snapshot", "[Snapshot]") {
    SnapshotManager manager;
    manager.addSnapshot("Original");
    manager.duplicateSnapshot(0);

    REQUIRE(manager.getSnapshotCount() == 2);
    REQUIRE(manager.getSnapshot(1).name == "Original");
    REQUIRE(manager.getActiveIndex() == 1);
}

TEST_CASE("Can remove snapshot", "[Snapshot]") {
    SnapshotManager manager;
    manager.addSnapshot("First");
    manager.addSnapshot("Second");
    manager.removeSnapshot(0);

    REQUIRE(manager.getSnapshotCount() == 1);
    REQUIRE(manager.getSnapshot(0).name == "Second");
}

TEST_CASE("Clear recall safe resets all flags", "[Snapshot]") {
    SnapshotManager manager;
    manager.addSnapshot("Test");

    // Manually set a recall safe flag (via direct access)
    auto& snap = const_cast<SnapshotData&>(manager.getSnapshot(0));
    std::fill(std::begin(snap.recallSafe), std::end(snap.recallSafe), true);

    manager.clearRecallSafe();

    // Check it was reset
    REQUIRE(std::all_of(std::begin(snap.recallSafe), std::end(snap.recallSafe),
                       [](bool b) { return !b; }));
}