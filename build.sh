#!/bin/bash
# SuperRack Linux - Build script for Linux x86-64

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}SuperRack Linux - Build Script${NC}"
echo ""

# Check dependencies
echo "Checking dependencies..."

# CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: cmake not found${NC}"
    exit 1
fi
echo -e "${GREEN}✓ cmake $(cmake --version | head -1)${NC}"

# C++ compiler (GCC or Clang)
CXX=""
if command -v g++ &> /dev/null; then
    CXX="g++"
elif command -v clang++ &> /dev/null; then
    CXX="clang++"
fi

if [ -z "$CXX" ]; then
    echo -e "${RED}Error: no C++ compiler found (need g++ or clang++)${NC}"
    exit 1
fi
echo -e "${GREEN}✓ $CXX${NC}"

# Check C++20 support
$CXX --version | head -1

# Check if JUCE is present
JUCE_DIR="${SCRIPT_DIR}/JUCE"
if [ -d "$JUCE_DIR" ]; then
    echo -e "${GREEN}✓ JUCE found at $JUCE_DIR${NC}"
else
    echo -e "${YELLOW}⚠ JUCE not found. Cloning from GitHub...${NC}"
    git clone --recurse-submodules https://github.com/juce-framework/JUCE.git "${JUCE_DIR}"
    cd "${JUCE_DIR}"
    git checkout 8.0.4
    cd "${SCRIPT_DIR}"
    echo -e "${GREEN}✓ JUCE cloned${NC}"
fi

# Check Lilv
if pkg-config --exists lilv-0 >= 0.24; then
    LILV_VERSION=$(pkg-config --modversion lilv-0)
    echo -e "${GREEN}✓ Lilv $LILV_VERSION found${NC}"
else
    echo -e "${YELLOW}⚠ Lilv not found. Install: sudo apt install liblilv-dev${NC}"
    echo -e "${YELLOW}  Or: sudo dnf install lilv-devel${NC}"
    read -p "Continue anyway? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Check ALSA (optional)
if pkg-config --exists alsa; then
    echo -e "${GREEN}✓ ALSA found${NC}"
else
    echo -e "${YELLOW}⚠ ALSA not found. Install: sudo apt install libasound2-dev${NC}"
fi

echo ""
echo "Building..."
echo ""

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DJUCE_DIR="${JUCE_DIR}" \
    $@

# Build
cmake --build . --config Release -j$(nproc)

echo ""
echo -e "${GREEN}Build complete!${NC}"
echo ""
echo "Run with:"
echo "  ./superRackLinux [session.json]"
echo ""
echo "Run tests:"
echo "  cd tests && ./superRackTests"
echo ""

# Try to run audio check
echo "Checking audio devices..."
if command -v aplay &> /dev/null; then
    echo "Available audio devices:"
    cat /proc/asound/cards 2>/dev/null || echo "  (no /proc/asound)"
    echo ""
    echo " ALSA devices:"
    aplay -l 2>/dev/null | head -20 || echo "  (none found)"
fi