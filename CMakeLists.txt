cmake_minimum_required(VERSION 3.20)
project(superRackLinux
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "Linux-аналог Waves SuperRack Performer для живой обработки звука"
)

# C++20 required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Detect JUCE_DIR if not set
if(NOT DEFINED JUCE_DIR)
    # Try common locations
    if(EXISTS "${CMAKE_SOURCE_DIR}/JUCE")
        set(JUCE_DIR "${CMAKE_SOURCE_DIR}/JUCE" CACHE PATH "Path to JUCE source")
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/../JUCE")
        set(JUCE_DIR "${CMAKE_SOURCE_DIR}/../JUCE" CACHE PATH "Path to JUCE source")
    endif()
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -fdiagnostics-color=always
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compile_options(-fno-unsafe-math-optimizations)
    endif()
endif()

# Thread safety annotations (CMake 3.17+)
set(THREADS_HAVE_STD_ATOMIC_FLAG_IN_TRHEAD 1)
find_package(Threads REQUIRED)

# JUCE (module-based)
if(JUCE_DIR AND EXISTS "${JUCE_DIR}/JUCEModules/JUCEHeader.h")
    message(STATUS "Using JUCE from: ${JUCE_DIR}")
    list(APPEND CMAKE_MODULE_PATH "${JUCE_DIR}/CMake")
    add_subdirectory(JUCE EXCLUDE_FROM_ALL)
    # JUCE as interface library
    add_library(JUCE INTERFACE)
    target_include_directories(JUCE INTERFACE "${JUCE_DIR}/JUCEModules")
else()
    message(FATAL_ERROR "JUCE not found. Set JUCE_DIR or clone JUCE alongside this project.")
endif()

# Lilv (LV2 host library)
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(LILV QUIET lilv>=0.24)
endif()
if(NOT LILV_FOUND)
    # Fallback: try pkg-config directly
    pkg_check_modules(LILV REQUIRED lilv>=0.24)
endif()
message(STATUS "Lilv version: ${LILV_VERSION}")
message(STATUS "Lilv include: ${LILV_INCLUDE_DIRS}")
message(STATUS "Lilv libs: ${LILV_LIBRARIES}")

# ALSA (optional, for X32 USB audio)
find_package(ALSA QUIET)

# nlohmann_json for session/preset storage
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(json)

# Application target
add_executable(superRackLinux
    src/app/Main.cpp
    src/app/App.cpp
    src/audio/AudioEngine.cpp
    src/audio/DeviceManager.cpp
    src/routing/RoutingMatrix.cpp
    src/racks/RackProcessor.cpp
    src/plugins/LV2PluginHost.cpp
    src/session/SessionManager.cpp
    src/snapshots/SnapshotManager.cpp
    src/midi/MIDIProcessor.cpp
    src/groups/GroupManager.cpp
    src/metering/Metering.cpp
)

target_include_directories(superRackLinux PRIVATE
    src/
    src/app/
    src/audio/
    src/routing/
    src/racks/
    src/plugins/
    src/session/
    src/snapshots/
    src/midi/
    src/groups/
    src/metering/
    src/ui/
    ${JUCE_DIR}/JUCEModules
    ${LILV_INCLUDE_DIRS}
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
)

target_link_libraries(superRackLinux PRIVATE
    JUCE::juce_audio_devices
    JUCE::juce_audio_formats
    JUCE::juce_audio_processors
    JUCE::juce_audio_utils
    JUCE::juce_core
    JUCE::juce_data_structures
    JUCE::juce_events
    JUCE::juce_graphics
    JUCE::juce_gui_basics
    JUCE::juce_gui_extra
    Threads::Threads
    ${LILV_LIBRARIES}
    nlohmann_json::nlohmann_json
)

if(ALSA_FOUND)
    target_link_libraries(superRackLinux PRIVATE ${ALSA_LIBRARY})
    target_compile_definitions(superRackLinux PRIVATE SR_HAVE_ALSA=1)
endif()

# Install
install(TARGETS superRackLinux DESTINATION bin)

# Tests
enable_testing()
add_subdirectory(tests)