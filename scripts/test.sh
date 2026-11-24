#!/bin/zsh
set -euo pipefail
cd "${0:A:h}/.."
export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
xcodebuild -project CS80.xcodeproj -scheme CS80Tests -configuration Release -derivedDataPath build build
mkdir -p output/audio output/validation
build/Build/Products/Release/CS80Tests output/audio/cs80-demo.wav | tee output/validation/offline-tests.txt
