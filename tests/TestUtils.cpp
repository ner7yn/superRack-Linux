#include <catch2/catch_test_macros.hpp>
#include "../src/common/Utils.h"
#include <cmath>

using namespace sr::utils;

TEST_CASE("DB to Gain conversion", "[Utils]") {
    // 0 dB = 1.0 gain
    REQUIRE(FloatsEqual(dbToGain(0.0f), 1.0f, 0.0001f));

    // +6 dB ≈ 2.0 gain
    REQUIRE(FloatsEqual(dbToGain(6.0f), 2.0f, 0.01f));

    // -6 dB ≈ 0.5 gain
    REQUIRE(FloatsEqual(dbToGain(-6.0f), 0.5f, 0.01f));
}

TEST_CASE("Gain to DB conversion", "[Utils]") {
    REQUIRE(FloatsEqual(gainToDb(1.0f), 0.0f, 0.0001f));
    REQUIRE(FloatsEqual(gainToDb(2.0f), 6.0f, 0.01f));
    REQUIRE(FloatsEqual(gainToDb(0.5f), -6.0f, 0.01f));
}

TEST_CASE("UUID generation produces unique values", "[Utils]") {
    auto uuid1 = generateUUID();
    auto uuid2 = generateUUID();
    REQUIRE(uuid1 != uuid2);
}

TEST_CASE("Timestamp is valid format", "[Utils]") {
    auto ts = timestampNow();
    REQUIRE(ts.isNotEmpty());
    REQUIRE(ts.containsChar('-'));
    REQUIRE(ts.containsChar(':'));
}