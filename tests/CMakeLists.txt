cmake_minimum_required(VERSION 3.20)
project(superRackLinuxTests LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(catch2)

enable_testing()

# Test executable
add_executable(superRackTests
    tests/TestUtils.cpp
    tests/TestMetering.cpp
    tests/TestRoutingMatrix.cpp
    tests/TestRackProcessor.cpp
    tests/TestSnapshotManager.cpp
)

target_include_directories(superRackTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/
    ${CMAKE_SOURCE_DIR}/src/common/
    ${CMAKE_BINARY_DIR}/_deps/fetchcontent-src/
)

target_link_libraries(superRackTests PRIVATE
    Catch2::Catch2WithMain
    nlohmann_json::nlohmann_json
)

add_test(NAME UtilsTests COMMAND superRackTests "[Utils]")
add_test(NAME MeteringTests COMMAND superRackTests "[Metering]")
add_test(NAME RoutingTests COMMAND superRackTests "[Routing]")
add_test(NAME RackTests COMMAND superRackTests "[Rack]")
add_test(NAME SnapshotTests COMMAND superRackTests "[Snapshot]")