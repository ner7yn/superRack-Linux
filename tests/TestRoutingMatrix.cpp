#include <catch2/catch_test_macros.hpp>
#include "../src/routing/RoutingMatrix.h"
#include "../src/racks/RackProcessor.h"
#include "../src/plugins/LV2PluginHost.h"

using namespace sr;

TEST_CASE("Routing matrix basic setup", "[Routing]") {
    RoutingMatrix routing;
    auto routingData = routing.getRouting();
    REQUIRE(routingData.empty());
}

TEST_CASE("Routing sets channel to rack mapping", "[Routing]") {
    RoutingMatrix routing;
    RoutingEntry entry;
    entry.physicalInput = 0;
    entry.programChannel = 0;
    entry.rackId = 0;
    entry.physicalOutput = 0;
    entry.mode = RackMode::Mono;

    std::vector<RoutingEntry> routingVec = {entry};
    routing.setRouting(routingVec);

    REQUIRE(routing.getRackForChannel(0) == 0);
    REQUIRE(routing.getInputForChannel(0) == 0);
    REQUIRE(routing.getOutputForRack(0) == 0);
}

TEST_CASE("Routing handles multiple channels", "[Routing]") {
    RoutingMatrix routing;
    std::vector<RoutingEntry> entries;

    for (int i = 0; i < 8; ++i) {
        RoutingEntry entry;
        entry.physicalInput = i;
        entry.programChannel = i;
        entry.rackId = i;
        entry.physicalOutput = i;
        entry.mode = RackMode::Mono;
        entries.push_back(entry);
    }

    routing.setRouting(entries);

    for (int i = 0; i < 8; ++i) {
        REQUIRE(routing.getRackForChannel(i) == i);
        REQUIRE(routing.hasRoute(i));
    }
}

TEST_CASE("Routing is empty after reset", "[Routing]") {
    RoutingMatrix routing;
    RoutingEntry entry;
    entry.physicalInput = 0;
    entry.programChannel = 0;
    entry.rackId = 0;
    entry.physicalOutput = 0;
    entry.mode = RackMode::Mono;

    routing.setRouting({entry});
    routing.reset();

    REQUIRE(routing.getRackForChannel(0) == -1);
    REQUIRE(!routing.hasRoute(0));
}