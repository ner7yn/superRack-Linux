#include <catch2/catch_test_macros.hpp>
#include "../src/metering/Metering.h"
#include <vector>

using namespace sr;

TEST_CASE("Metering processes silence correctly", "[Metering]") {
    Metering metering;
    metering.setNumChannels(2);

    std::vector<float> buffer(1024, 0.0f);
    std::vector<const float*> channels = {buffer.data(), buffer.data()};

    metering.processBlock(channels.data(), 2, 1024);

    REQUIRE(metering.getInputPeak(0) == 0.0f);
    REQUIRE(metering.getInputPeak(1) == 0.0f);
}

TEST_CASE("Metering detects peak level", "[Metering]") {
    Metering metering;
    metering.setNumChannels(1);

    std::vector<float> buffer(1024, 0.8f);
    std::vector<const float*> channels = {buffer.data()};

    metering.processBlock(channels.data(), 1, 1024);

    REQUIRE(metering.getInputPeak(0) > 0.7f);
    REQUIRE(metering.getInputPeak(0) < 0.9f);
}

TEST_CASE("Metering detects clipping", "[Metering]") {
    Metering metering;
    metering.setNumChannels(1);

    std::vector<float> buffer(1024, 1.2f); // Above 1.0 = clipping
    std::vector<const float*> channels = {buffer.data()};

    metering.processBlock(channels.data(), 1, 1024);

    REQUIRE(metering.getInputClipped(0) == true);
}

TEST_CASE("Metering reset works", "[Metering]") {
    Metering metering;
    metering.setNumChannels(2);

    std::vector<float> buffer(1024, 0.8f);
    std::vector<const float*> channels = {buffer.data(), buffer.data()};

    metering.processBlock(channels.data(), 2, 1024);
    metering.reset();

    REQUIRE(metering.getInputPeak(0) == 0.0f);
    REQUIRE(metering.getInputPeak(1) == 0.0f);
}

TEST_CASE("Overall peak returns max of all channels", "[Metering]") {
    Metering metering;
    metering.setNumChannels(3);

    std::vector<float> buffers[3];
    buffers[0] = std::vector<float>(1024, 0.5f);
    buffers[1] = std::vector<float>(1024, 0.9f);
    buffers[2] = std::vector<float>(1024, 0.3f);
    std::vector<const float*> channels = {buffers[0].data(), buffers[1].data(), buffers[2].data()};

    metering.processBlock(channels.data(), 3, 1024);

    REQUIRE(metering.getOverallInputPeak() >= 0.85f);
    REQUIRE(metering.getOverallInputPeak() <= 0.95f);
}