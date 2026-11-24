# cs80maths

A free synthesizer from Direct Grooves, supplied as a Windows x64 VST3 instrument and native macOS AUv3 instrument and standalone application, built in Xcode with an Objective-C++ audio interface and a C++17 synthesis engine. All waveforms are generated mathematically. No recorded audio or waveform samples are used by the instrument.

This is a working first implementation of the documented architecture, **not yet a calibrated replica**. Read [the accuracy report](docs/ACCURACY.md) for the specific approximations. The supplied Yamaha manuals remain the source reference; no Yamaha code or commercial plugin assets are used.

Version 0.3.0 adds Windows VST3 and uninstallers for both platforms. Version 0.2.1 smooths pressure, pitch bend, continuous MIDI controls and filter changes, and fixes pedal-release discontinuities. Version 0.2.0 added a CS-80-style panel with colored vertical faders, a memory-cover editor, and eight original Vangelis-inspired performance presets. The click fixes from 0.1.1 are retained. Quit an older running app and unload/reload the AU instance after updating.

## Open and play

Quit any earlier test instance first. Open `dist/cs80maths.app`, click **Start Audio**, and choose your MIDI input in the toolbar. The application uses the Mac's current default audio output. Audio is stopped on launch so opening the app does not immediately play sound.

You can also click the on-screen keyboard. Click it first to give it focus, then use `A W S E D F T G Y H U J K O L P ;` for chromatic notes. The ribbon measures movement relative to where you first touch it and returns when released. A pressure-capable trackpad can send pressure while a mouse-held note is active.

The two rows of **Tone Selector** buttons select presets, memories, or the corresponding programming panel. Select **Panel** before editing the Channel I/II sliders. Memories 1/3 belong to I; memories 2/4 belong to II. Open **Memory Bank** or **Open Cover** to edit the four memories. The memory copy buttons are a software convenience. Standalone Save/Load writes and restores all 204 parameters, including the six physical-style panels. A host saves the same state with its project.

## Use as an Audio Unit

Keep the app bundle intact: it contains `Contents/PlugIns/CS80AudioUnit.appex`. Launch the containing app once so macOS can discover the extension, then reopen or rescan Audio Units in your host. The component is **Direct Grooves: cs80maths**, type `aumu`, subtype `Cs80`, manufacturer `DjAz`. AUv3 is an app extension; there is no separate AUv2 `.component` file to copy into the Components folder.

The local build is ad hoc signed. It is intended for this development machine, not notarized distribution. A distributable release needs your Apple signing identity and notarization. No developer account credentials have been added to the project.

The AU exposes a mono/stereo audio input for **external modulation**. With the suboscillator Function set to External, the first input channel becomes the modulation signal. It does not directly mix an external instrument into the audio output. Host support for routing an input into an AU instrument varies. The standalone app does not capture a microphone or expose an external-input routing selector.

## Akai APC64

The APC64 supports pad velocity and polyphonic aftertouch. Leave this instrument's **MPE input off** when using ordinary APC64 notes and poly pressure. The per-pad pressure message affects that note's two layers independently of other held notes.

In Note/Chord configuration, enable **Send Poly Aftertouch**. Akai's v1.1 guide locates that toggle at row 5, column 5, accessed from the Note/Chord configuration. In a custom pad layout, enable **Poly AT** for the pads and send them to USB. Use the musical input port carrying notes, rather than assuming every APC64 port carries pad performance. In the standalone app, select that port from the toolbar. In a DAW, enable that input and arm/monitor the instrument track.

For the eight touch faders in a custom controller layout, assign CC16 through CC23 in this order:

| MIDI CC | Instrument control |
| --- | --- |
| 16 | Overall brilliance |
| 17 | Overall resonance |
| 18 | Mix I / II |
| 19 | Suboscillator VCO amount |
| 20 | Suboscillator VCF amount |
| 21 | Ring modulation amount |
| 22 | Ring carrier speed, exponential range |
| 23 | Expression |

These are explicit software mappings, not a claim that every APC64 factory mode transmits these CCs. The project does not change your APC64 configuration or send commands to its hardware. In a host, you can instead map the faders to any of the exposed Audio Unit parameters using the host's mapping features.

Akai references: [official FAQ](https://support.akaipro.com/en/support/solutions/articles/69000844959-akai-pro-apc-64-frequently-asked-questions), [User Guide v1.1, printed pages 26–29 and 51–53](https://cdn.inmusicbrands.com/akai/APC64/APC64%20-%20User%20Guide%20-%20v1.1.pdf). The APC64 Notes input was available and selected during the editor check. Physical pad/fader response comparison remains to be done.

## Performance presets

The menu above the faders includes Horizon Brass, Neon Rain, Aegean Strings, Distant Choir, Glass Bells, Solar Flare, Velvet Lead, and Low Horizon. These program both layers and pressure response. They are original inspired sounds rather than exact artist patches. They are also in the host preset menu and exported under `output/presets/`. See [the performance-bank guide](docs/PERFORMANCE-PRESETS.md) for playing notes and memory/state behavior.

## Other MIDI

Polyphonic key pressure, channel-pressure fallback, note-on/off, velocity, and pitch bend are supported. Pitch bend is ±2 semitones in ordinary MIDI mode. The optional lower-zone MPE mode uses channel 1 as master and channels 2–16 as members with ±48-semitone member bend. This is a software compatibility mode, not original CS-80 hardware behavior. MPE CC74 is deliberately unmapped.

CC1 controls common suboscillator pitch depth; CC7 master volume; CC11 expression; CC64 the binary foot switch. Receiving CC64 marks the virtual switch as connected. With no switch connected, the assigner path behaves as the original's closed jack contact. The main panel exposes both the switch state and connection state. Sustain has a timed die-away; CC64 is not an infinite damper latch. CC120 is all sound off; CC123 releases held notes.

## Build in Xcode

Open `CS80.xcodeproj`, choose **CS80**, and run on **My Mac**. Deployment target: macOS 13 or later. The project has no downloaded dependencies. Xcode 27.2 was used for this build.

```sh
./scripts/build.sh
./scripts/test.sh
```

The build script produces a universal Apple silicon/Intel app in `dist/`. The test script builds the native offline test executable and writes its report and an eight-second demonstration render under `output/`. That WAV is an output of the synthesis engine, not an instrument resource.

For OS-level validation after the app has been discovered:

```sh
auval -v aumu Cs80 DjAz
build/Build/Products/Release/CS80Tests --host
build/Build/Products/Release/CS80Tests --standalone
```

These need access to macOS audio and extension services; a restrictive shell sandbox may prevent discovery even when the extension is valid. `--host` checks out-of-process AUv3 loading, custom editor creation, MIDI delivery, and offline host-engine audio. `--standalone` checks preset selection through the in-process AVAudioUnit host API. The **CS80Tests** Xcode scheme runs the same offline executable. This is a command-line test runner, not an XCTest bundle.

## Source layout

`CS80/DSP` contains the parameter definitions, initial factory approximations, procedural oscillators, envelopes, filters, voice allocation, MIDI handling, and output effects. `CS80/AudioUnit` exposes the render callback, parameter tree, host state, buses, presets, and MIDI event handling. `CS80/UI` contains the native editor, keyboard, and relative ribbon. `CS80/App` contains the standalone audio/MIDI host. `CS80/Tests` contains offline and registered-host checks.

[Text-only electronic reference](docs/cs80-electronics.html) · [Manual study notes](docs/CS80-study-notes.txt) · [Accuracy and differences](docs/ACCURACY.md)

## Windows and release publishing

See [Windows instructions](docs/WINDOWS.md). The GitHub Actions workflow compiles and validates Windows builds remotely. No Windows tools need installing on a Mac. Both binary downloads include an uninstaller.

Build and test the macOS release with scripts/build.sh and scripts/test.sh. Download the successful Windows workflow artifact to dist/releases, then run scripts/package_release.py --windows dist/releases/cs80maths-0.3.0-Windows-x64-VST3.zip --site /Users/djazz/code/directgrooves-finall/new. Check the local site before running its deploy.sh. Release metadata and assets are staged locally; packaging never edits the live site directly.
