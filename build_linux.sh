#!/usr/bin/env bash
set -e

echo "============================================================"
echo "  Building IRON SMASH - Rock/Metal Drum Bus Enhancer (VST3)"
echo "============================================================"

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel

echo ""
echo "[SUCCESS] Linux VST3 built successfully!"
echo "Location: build/RockDrumEnhancer_artefacts/Release/VST3/Iron Smash Drum Enhancer.vst3"
echo "To install, copy it to: ~/.vst3/"
