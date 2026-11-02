#!/usr/bin/env bash
set -e

echo "============================================================"
echo "  Building IRON SMASH - Rock/Metal Drum Bus Enhancer (VST3)"
echo "============================================================"

cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build build --config Release --parallel

echo ""
echo "[SUCCESS] Universal macOS VST3 built successfully!"
echo "Location: build/RockDrumEnhancer_artefacts/Release/VST3/Iron Smash Drum Enhancer.vst3"
echo "To install, copy it to: ~/Library/Audio/Plug-Ins/VST3/"
