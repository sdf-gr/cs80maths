# Accuracy and known differences

The first build is playable and host-valid, but it is not an exact sonic clone. This document separates architecture reproduced in software from circuit behavior that remains approximate. Passing an Audio Unit validation test establishes software/host behavior, not a match to a vintage instrument.

## Implemented signal relationships

- Eight simultaneous assignments, each driving two complete synthesis layers: sixteen procedural oscillators, separate HPF/LPF chains, filter envelopes, and amplitude envelopes.
- Pulse and saw selected independently, white noise introduced before filtering, and a phase-coherent sine contribution bypassing the filters but following amplitude shaping.
- Pulse width and bank-shared PWM; a separate common suboscillator; a separate common ring carrier/envelope; a separate stereo-effect modulation system.
- Filter IL/AL attack/decay/reference/release structure. On release the filter envelope moves toward IL rather than an unrelated zero cutoff.
- Channel initial/after brilliance and level. Initial bend; pressure-controlled modulation; keyboard-position controls; separate footages and channel-II detune.
- Eight held notes take priority; a ninth is ignored. Repeated key strikes rotate through the available assignments. Released assignments can be reused. A preallocated transition pool fades their old audio for 0.5 ms during reuse; this does not add held-key assignments.
- Timed Sustain I and Sustain II behavior, switch assignment/unplugged semantics, continuous glide and semitone glissando, sustain-dependent ribbon/glissando continuation.
- Shared ring-envelope trigger on transition from no held keys to a new phrase, rather than per-note retrigger while other keys remain held.
- Two physical-style programming panels and four memory panels. All their values are saved in host state even while another sound is selected.

## Models that are approximations

**Factory presets.** The twenty-two tone names are present, but the initial values in `Parameters.hpp` are manual-informed starting points. They have not been derived as exact T51–T54 matrix voltages. Some related voices intentionally share a starting model, so they are not yet the distinct calibrated factory tones. This is the largest immediate limitation for preset comparisons.

**Oscillator and waveshaper.** Saw and pulse use polyBLEP correction and 2× processing; sine is evaluated analytically. The original IG00153/IG00158 waveform asymmetries, reset transients, triangle shaping, DC shifts and residual harmonics are not transistor-modeled. Oscillators continue to advance while idle. Stable per-voice offsets provide modest variation; thermal drift, aging, supply interactions and measured voice-to-voice tolerance distributions are not modeled.

**Filters.** Each layer uses two-pole TPT state-variable high-pass and low-pass stages with independent resonance and key tracking. Soft saturation precedes the filter chain. This is a stable musical approximation to the documented response, not a solved IG00156 transistor/macromodel. The resonance limits, internal saturation, noise, cutoff law and actual control-voltage mixing ratios need calibration. Filter targets update every sixteen output samples; the TPT integrator parameters interpolate at the internal 2× sample rate.

**Envelopes and touch.** Attack is nominally 1 ms–1 s, decay/release 10 ms–10 s with exponential control mapping and curved segments. The original IC charging laws and slider tapers are unmeasured. Initial-touch decay is currently approximately 120 ms; brightness and pitch scales are chosen approximations. Pressure-speed combination currently uses the greatest pressure among held notes. Per-note pressure depth is independent, but the original common-speed combining circuit still needs analysis. Seven-bit MIDI pressure/velocity are less continuous than analog key sensors.

**Pitch and keyboard controls.** Coarse/fine and detune have documented nominal ranges. The LOW/HIGH keyboard weight curves, root tuning, and control-voltage scaling need matching to hardware. The oscillator/control system is represented in logarithmic musical pitch rather than simulating the original linear key-voltage ladder electrically.

**Glide and allocation.** Continuous pitch traverses sixty semitones in the selected full-range time. Each assignment retains its preceding pitch; glissando quantizes the traversed pitch to semitones. Exact YM26600/YM26700 scheduling, voice reuse, simultaneous-key scan priority, portamento integration, repeated-note overlap, and pedal/release edge cases have not been emulated at the original digital clock level. It is a functional model, not a cycle-accurate recreation.

**Sustain.** Performance sustain is combined with the channel release by using the longer active release. Assigned pedal cancellation uses a brief release rather than abruptly discontinuing the waveform. Sustain II retires previously released tails on the next note, with a 0.5 ms fade to avoid a discontinuity. The precise analog interaction between performance sustain, SH hold behavior and both EGs still needs circuit/hardware calibration.

**Ribbon.** First-touch-relative mouse movement, asymmetric upward/downward frequency travel and sustain-mode-dependent continuation are represented. The current normalized displacement uses a linear frequency ratio, with +1 doubling and −1 approaching zero. The original resistive strip's measured law and multi-finger contact behavior are not reproduced. An ordinary APC64 fader does not provide the same contact-reference information; the fixed CC map does not pretend it is the original ribbon.

**Ring modulation.** The mixed signal is blended with a sine-product ring signal. Carrier range is currently 0.5–2000 Hz, with an envelope that scales the frequency by up to five octaves. Those endpoints and scales are provisional. MA796 balance, feedthrough, overload and exact carrier-envelope control laws are not modeled.

**Tremolo/chorus.** A modulated interpolated delay plus complementary stereo amplitude changes produces the documented class of effect. It is not an MN3001/BA617 clocked-charge model: BBD clock leakage, bandwidth changes, companding if any, exact filters, modulation phasing and switching transitions are not reproduced. Chorus/tremolo uses one mode selector; the original independent buttons permit a combined setting that this UI does not expose. Engaging the effect produces a small level reduction.

**Output and expression.** Gain, expression/wah and the −20 dB range are functional controls. The optically sensed pedal taper, exact output-stage coloration, source impedance, headphone amplifier and power-supply behavior are not electrically simulated. There is no added reverb or sampled effect. Signal levels use a practical digital calibration rather than claiming a specified number of volts at a DAC output.

## Audio and host implementation

Fresh notes receive a 0.5 ms raised-cosine onset edge, and reused assignments receive a 0.5 ms old/new transition. These are software protections against discontinuities, not measured original-circuit timing. The amplitude envelope still reaches its peak at the 1 ms minimum setting. Frozen filter history is cleared on a new assignment while its retiring copy finishes the transition.

The real-time render path uses preallocated storage and does not load files, allocate voices dynamically, take locks, or call AppKit. A 63-tap low-pass FIR precedes 2:1 decimation. Its 15.5-output-sample group delay is reported as Audio Unit latency. This reduces aliasing but does not guarantee alias-free audio-rate modulation at all extreme settings. A light output low-pass and DC blocker provide conditioning; no hard limiter is inserted.

MIDI and scheduled parameter events are split at their host sample offsets. Explicit parameter ramps retain their progress across buffers. Ordinary UI changes and continuous MIDI CC changes are smoothed over about 5 ms. Pressure and pitch bend use a 2 ms time constant. Envelope duration changes restart the current stage from its present level. Discrete selectors change immediately. Zero/oversized buffers, owned output buffers, mono/stereo output and external-input routing are handled by the AU wrapper. The optional input's sample rate must match the output's rate.

The app uses the default audio output. It has no audio-device selector, recording transport, or standalone external-input capture. The editor now follows the hardware’s dual programming rows, colored vertical faders, tone-selector rows, ribbon, keyboard, and grouped performance controls. Memory editing and MIDI settings open in additional panels. It remains a native scalable interpretation, not a photograph or dimensional replica. The app's Start/Stop Audio replaces the physical power switch; no warm-up wait is imposed.

AUv3 is implemented; there is no AUv2, VST3, AAX or iOS build. The local app is ad hoc signed. Developer ID signing/notarization is a separate distribution step. No changes were made to the user's controller configuration or DAW projects.

## What would establish a closer match

Capture a calibrated CS-80's oscillator outputs, filter sweeps at several levels and resonances, envelope traces, pressure/velocity response, ribbon travel, preset control conditions, ring carrier curves, and stereo-effect impulse/modulation behavior. Match individual sections before matching full performances. The supplied schematics guide that work but do not replace measured nonlinear behavior. An APC64 can test the performance interface, but cannot supply measurements of the original analog circuits.

## Verification

See `output/validation` for the current build's checks. Offline tests cover tuning, bypass routing, note capacity/allocation, poly pressure, sustain modes and pedal cancellation, glide timing, filter-envelope release, external modulation, parameter ramps, multiple sample rates, memory routing, randomized parameter extremes, AU state recall, event sample offsets, and output-buffer handling. Apple `auval` and an out-of-process host/editor test provide separate host-level verification. These checks do not establish sonic equivalence.

## Version 0.2.0 performance bank

Eight original cinematic/Vangelis-inspired sounds program the two main panels and global sound settings. They are distinct from the twenty-two uncalibrated hardware-tone approximations. They are not claimed to reproduce any artist’s exact patch or recorded production chain. Their host preset numbers append to the existing list; no parameter addresses were changed. See `PERFORMANCE-PRESETS.md`.
