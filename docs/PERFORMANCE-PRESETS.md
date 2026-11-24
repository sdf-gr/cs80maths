# Cinematic performance bank

Eight original Vangelis-inspired patches are included in version 0.2.0. They are designed for the current procedural engine and polyphonic pressure. They are not recovered settings from Vangelis's instruments or copies of recordings. No sampled sounds, convolution IRs, or added reverb are used.

Choose a sound from **Performance Presets** above the main panel. The same sounds appear after the original eleven entries in the host's factory-preset menu. Loading from either menu programs both channel panels and selects PANEL for both tone-selector rows. The colored faders display the loaded settings and remain editable.

| Preset | Character | Playing approach |
| --- | --- | --- |
| Horizon Brass | Warm layered brass, filter opening, slight initial pitchbend | Play sustained chords; press individual pads harder for brightness and vibrato. |
| Neon Rain | Dark PWM pad with a slower, brighter upper layer | Let chords develop. Pressure brings the second layer forward in tone. |
| Aegean Strings | Broad detuned pulse/saw strings with gentle stereo movement | Use medium velocities and long held chords. |
| Distant Choir | Resonant, vocal-like bandpass layers with a fifth-related upper voice | Hold low or middle-register chords and add pressure sparingly. |
| Glass Bells | Sine-led bells with a shared ring-modulator sweep and long decay | Release all notes between phrases to restart the ring envelope. |
| Solar Flare | Warm foundation with a slower octave layer and a bright swell | Hold notes long enough for the second layer to bloom. |
| Velvet Lead | Rounded pulse/saw lead with initial pitchbend and pressure vibrato | Use short phrases and pressure at the ends of notes. |
| Low Horizon | Low sine foundation with a restrained, moving upper layer | Play in the bass/middle registers; leave space between harmonies. |

For APC64 pad notes and polyphonic aftertouch, leave MPE off and enable Send Poly Aftertouch on the controller. The main **After VCO/VCF** controls and each channel's **After Brilliance/Level** amounts determine expression. The physical controller's force curve is not the same as a CS-80 keyboard, so these amounts remain useful adjustments.

The presets leave the user's four memory panels, MPE selection, foot-switch connection/state, live expression value, and output range intact. They intentionally change sound controls such as filter, modulation, footage, detune, envelope, and effect settings. The original host preset numbers 0–10 and all 204 parameter addresses remain unchanged; the new sounds use factory preset numbers 11–18.

Exported `.aupreset` files are in `output/presets/`. They can be loaded through the standalone app's **Load sound…** button or a host that imports Audio Unit presets. These files are complete state snapshots, including the memory-panel values present during export. Use the Performance Presets menu when you want to keep your current memories and controller settings intact.

**Memory Bank / Open Cover** opens the four memory panels. Choose a memory, edit its faders, copy from its associated channel panel if needed, and use **Select This Memory** to route it into the corresponding bank. Close the cover to return to both main programming rows.

The editor follows the CS-80's functional panel arrangement and fader colors, with software additions for preset browsing, memory copying, numerical readouts, window scaling, and MIDI settings. It is not a dimensional or photographic reproduction. The separate sonic-calibration limitations in `ACCURACY.md` still apply.
