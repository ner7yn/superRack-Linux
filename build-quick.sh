#!/bin/bash
# Quick build for testing on Linux

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

# Clone JUCE if needed
if [ ! -d "${SCRIPT_DIR}/JUCE" ]; then
    echo "Cloning JUCE..."
    git clone --depth 1 https://github.com/juce-framework/JUCE.git
    cd JUCE && git checkout 8.0.4 && cd ..
fi

# Create build dir
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure and build
cmake .. -DCMAKE_BUILD_TYPE=Release -DJUCE_DIR="${SCRIPT_DIR}/JUCE"
cmake --build . -j$(nproc)

echo "Build successful!"