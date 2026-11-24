# cs80maths for Windows — Direct Grooves

Windows 10/11 x64 VST3 instrument. A VST3-compatible DAW is required; this download does not include a standalone Windows app. The DSP is shared with the macOS version. The native Windows fader editor provides both voice panels, all four memory editors, performance-control groups, tone selectors and eight performance presets. Choose Panel or the corresponding Memory under Tone selectors to hear edits. Use your DAW's keyboard/MIDI input to play. Polyphonic pressure and channel pressure are supported; optional MPE has the same mapping as the macOS implementation.

## Install and uninstall

1. Close the DAW and extract the entire ZIP.
2. Run `Install cs80maths.cmd`. It installs the bundle in `%LOCALAPPDATA%\Programs\Common\VST3\Direct Grooves`, without administrator rights. An existing managed installation is kept as `cs80maths.vst3.previous`; move an earlier backup elsewhere before a second update.
3. Restart/rescan your DAW and load Direct Grooves / cs80maths. Some hosts need the user VST3 path added to their scan settings. You may instead manually copy the whole `cs80maths.vst3` directory into a VST3 location supported by your host.
4. **An uninstaller is included.** Close the DAW and run `Uninstall cs80maths.cmd` from the extracted download or installed Direct Grooves folder. Type YES when prompted. It removes only the managed plugin and leaves presets, settings, earlier backups and manually copied bundles alone. Remove manually installed copies from their original locations yourself.

This release is unsigned. Windows may identify its publisher as unknown. The scripts do not disable SmartScreen, antivirus or security settings. Only run software you trust. Windows ARM and 32-bit hosts are not supported by this x64 build.

## Build from free source

MIT license. The SDK's own MIT license is retained. Source builds need Visual Studio 2022 with C++ and CMake 3.25+ on Windows. The supplied GitHub Actions workflow builds on a remote Windows runner; no Windows SDK, compiler or VM needs installing on a Mac.

Fetch https://github.com/steinbergmedia/vst3sdk at commit `3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96` and initialize its base, cmake, pluginterfaces and public.sdk submodules. Configure with `cmake -S . -B win-build -A x64 -DVST3_SDK_ROOT=C:/path/to/vst3sdk`, build with `cmake --build win-build --config Release`, and run `ctest --test-dir win-build -C Release --output-on-failure`. The build also runs Steinberg's plugin validator. Package using `scripts/package_windows.ps1`.

The sound model is an uncalibrated approximation, not a component-exact or measured CS-80 replica. Tests on a hosted runner do not replace listening in your own DAW and audio device configuration.
