#!/bin/zsh
set -euo pipefail
cd "${0:A:h}/.."
export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
xcodebuild -project CS80.xcodeproj -scheme CS80 -configuration Release -derivedDataPath build 'ARCHS=arm64 x86_64' build
mkdir -p dist
/usr/bin/ditto 'build/Build/Products/Release/cs80maths.app' 'dist/cs80maths.app'
/usr/bin/codesign --verify --deep --strict 'dist/cs80maths.app'
