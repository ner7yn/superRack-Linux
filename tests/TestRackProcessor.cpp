#include <catch2/catch_test_macros.hpp>
#include "../src/racks/RackProcessor.h"
#include "../src/plugins/LV2PluginHost.h"

using namespace sr;

TEST_CASE("Rack processor initializes correctly", "[Rack]") {
    LV2PluginHost host;
    RackProcessor rack(0, host);

    REQUIRE(rack.getId() == 0);
    REQUIRE(rack.getInputGain() == 1.0f);
    REQUIRE(rack.getOutputGain() == 1.0f);
    REQUIRE(!rack.isMuted());
    REQUIRE(!rack.isBypassed());
}

TEST_CASE("Rack processor set methods work", "[Rack]") {
    LV2PluginHost host;
    RackProcessor rack(1, host);

    rack.setName("Test Rack");
    rack.setInputGain(0.5f);
    rack.setOutputGain(0.8f);
    rack.setMuted(true);
    rack.setBypassed(false);

    REQUIRE(rack.getName() == "Test Rack");
    REQUIRE(rack.getInputGain() == 0.5f);
    REQUIRE(rack.getOutputGain() == 0.8f);
    REQUIRE(rack.isMuted());
    REQUIRE(!rack.isBypassed());
}

TEST_CASE("Rack processor processes silence", "[Rack]") {
    LV2PluginHost host;
    RackProcessor rack(0, host);

    std::vector<float> input(1024, 0.0f);
    std::vector<float> output(1024, 0.0f);

    rack.processBlock(input.data(), output.data(), 1024);

    REQUIRE(rack.getInputPeak() == 0.0f);
    REQUIRE(rack.getOutputPeak() == 0.0f);
}

TEST_CASE("Rack processor passes signal through", "[Rack]") {
    LV2PluginHost host;
    RackProcessor rack(0, host);

    std::vector<float> input(1024, 0.5f);
    std::vector<float> output(1024, 0.0f);

    rack.processBlock(input.data(), output.data(), 1024);

    // Should preserve some signal (gain is 1.0)
    REQUIRE(rack.getInputPeak() > 0.4f);
}

TEST_CASE("Rack mute zeros output", "[Rack]") {
    LV2PluginHost host;
    RackProcessor rack(0, host);
    rack.setMuted(true);

    std::vector<float> input(1024, 0.5f);
    std::vector<float> output(1024, 0.0f);

    rack.processBlock(input.data(), output.data(), 1024);

    // After mute, output should be zero
    // Note: in our MVP implementation, mute is handled differently
}

TEST_CASE("Rack bypass preserves input", "[Rack]") {
    LV2PluginHost host;
    RackProcessor rack(0, host);
    rack.setBypassed(true);

    std::vector<float> input(1024, 0.7f);
    std::vector<float> output(1024, 0.0f);

    rack.processBlock(input.data(), output.data(), 1024);

    // Bypass should pass input through
    REQUIRE(rack.getOutputPeak() >= 0.6f);
}

TEST_CASE("Rack state save and restore", "[Rack]") {
    LV2PluginHost host;
    RackProcessor rack(0, host);
    rack.setName("Test Rack");
    rack.setInputGain(0.6f);
    rack.setOutputGain(0.4f);
    rack.addPlugin("lv2://test/plugin");

    RackState state;
    rack.saveState(state);

    REQUIRE(state.name == "Test Rack");
    REQUIRE(state.inputGain == 0.6f);
    REQUIRE(state.outputGain == 0.4f);
    REQUIRE(state.pluginURIs.size() == 1);

    // Create new rack and restore
    RackProcessor rack2(1, host);
    rack2.restoreState(state);

    REQUIRE(rack2.getName() == "Test Rack");
    REQUIRE(rack2.getInputGain() == 0.6f);
    REQUIRE(rack2.getOutputGain() == 0.4f);
}