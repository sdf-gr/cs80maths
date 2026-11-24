#pragma once
#include "Parameters.hpp"
namespace cs80 {
// Original performances inspired by cinematic CS-80 textures; not transcriptions
// of commercial recordings or claims about an artist's original settings.
constexpr int PerformancePresetCount=8;
inline const std::array<const char*,PerformancePresetCount> performanceNames={
 "Horizon Brass","Neon Rain","Aegean Strings","Distant Choir",
 "Glass Bells","Solar Flare","Velvet Lead","Low Horizon"};
inline const std::array<const char*,PerformancePresetCount> performanceNotes={
 "Expressive two-layer brass. Lean into pad pressure to open the tone and add vibrato.",
 "Dark, spacious synth pad with slow PWM and a brighter pressure-controlled upper layer.",
 "Broad, gently detuned strings. Hold chords and use independent pad pressure for movement.",
 "A soft, vocal-like bandpass pad with an octave layer and a slow stereo movement.",
 "Long-decaying sine bells with a shared ring carrier. Release all notes to retrigger its sweep.",
 "A bright cinematic swell: a warm foundation and a slower, pressure-sensitive upper voice.",
 "A rounded singing lead. Initial pitchbend and pressure vibrato supply the articulation.",
 "A deep, restrained bass pad with a sine foundation and a gently moving upper layer."};
struct PerformancePreset {std::array<float,ParameterCount>values;};
inline PerformancePreset performancePreset(int index){
 PerformancePreset sound;auto specs=makeSpecs();for(int i=0;i<ParameterCount;i++)sound.values[i]=specs[i].initial;
 auto&v=sound.values;
 v[ToneI]=v[ToneII]=13;v[Volume]=.65f;v[Mix]=.5f;v[Detune]=4;v[Variation]=.2f;
 v[AfterVCO]=.16f;v[AfterVCF]=.12f;v[SubSpeed]=5.1f;v[InitialBend]=.035f;
 v[SubVCO]=v[SubVCF]=v[SubVCA]=0;v[EffectOn]=0;v[Sustain]=0;v[Glide]=0;
 PatchValues a={.22f,.12f,.58f,0,1,0,0,.03f,.48f,.15f,.45f,.6f,.34f,.62f,.5f,1,.05f,.25f,.7f,.72f,.62f,.78f,.2f,.32f,.48f,.32f};
 PatchValues b=a;
 switch(std::clamp(index,0,PerformancePresetCount-1)){
 case 0:
  a[IL]=.6f;a[AL]=.68f;a[FilterAttack]=.4f;a[AmpAttack]=.29f;a[LPF]=.45f;
  b[Square]=1;b[Saw]=1;b[PWMDepth]=.18f;b[PW]=.61f;b[IL]=.4f;b[AL]=.45f;b[LPF]=.42f;b[AmpAttack]=.38f;b[AmpLevel]=.58f;
  v[Detune]=4.5f;v[InitialBend]=.055f;v[Mix]=.43f;break;
 case 1:
  a[Square]=1;a[Saw]=0;a[PWMDepth]=.5f;a[PWMSpeed]=.15f;a[PW]=.64f;a[LPF]=.36f;a[IL]=.15f;a[AL]=.23f;a[AmpAttack]=.9f;a[FilterAttack]=.94f;a[AmpSustain]=.85f;a[AmpRelease]=.9f;a[FilterRelease]=.9f;
  b=a;b[Saw]=1;b[Square]=0;b[LPF]=.47f;b[HPF]=.12f;b[AmpLevel]=.45f;b[AmpAttack]=.98f;b[AfterBrilliance]=.6f;
  v[Detune]=6;v[SubVCF]=.025f;v[AfterVCO]=.1f;v[EffectOn]=1;v[EffectDepth]=.25f;v[EffectSpeed]=.12f;v[Mix]=.38f;v[InitialBend]=0;break;
 case 2:
  a[Square]=1;a[Saw]=0;a[PW]=.62f;a[PWMDepth]=.55f;a[PWMSpeed]=.25f;a[IL]=.14f;a[AL]=.18f;a[LPF]=.57f;a[AmpAttack]=.76f;a[AmpRelease]=.81f;a[AmpSustain]=.92f;a[FilterAttack]=.72f;
  b=a;b[Saw]=1;b[PW]=.56f;b[PWMDepth]=.38f;b[PWMSpeed]=.31f;b[LPF]=.5f;b[AmpLevel]=.56f;b[AmpAttack]=.84f;
  v[Detune]=5.8f;v[SubVCO]=.012f;v[AfterVCO]=.11f;v[EffectOn]=1;v[EffectDepth]=.22f;v[EffectSpeed]=.16f;v[InitialBend]=0;break;
 case 3:
  a[Square]=1;a[Saw]=1;a[PW]=.68f;a[HPF]=.25f;a[LPF]=.49f;a[ResH]=.32f;a[ResL]=.4f;a[IL]=.12f;a[AL]=.19f;a[AmpAttack]=.89f;a[FilterAttack]=.87f;a[AmpRelease]=.9f;a[AmpSustain]=.85f;
  b=a;b[HPF]=.3f;b[LPF]=.43f;b[ResL]=.32f;b[AmpLevel]=.35f;b[Sine]=.16f;
  v[FeetII]=2;v[Detune]=-3;v[SubVCF]=.035f;v[SubSpeed]=.8f;v[AfterVCO]=.025f;v[AfterVCF]=.16f;v[EffectOn]=1;v[EffectDepth]=.28f;v[EffectSpeed]=.08f;v[Mix]=.32f;v[InitialBend]=0;break;
 case 4:
  a[Square]=a[Saw]=0;a[Sine]=1;a[FilterLevel]=0;a[IL]=a[AL]=0;a[AmpAttack]=.02f;a[AmpDecay]=.87f;a[AmpSustain]=0;a[AmpRelease]=.84f;a[AmpLevel]=.84f;a[AfterLevel]=.15f;
  b=a;b[Sine]=.65f;b[Square]=1;b[FilterLevel]=.12f;b[LPF]=.42f;b[AmpDecay]=.68f;b[AmpLevel]=.45f;
  v[FeetII]=3;v[Detune]=2;v[RingMod]=.63f;v[RingSpeed]=221;v[RingDepth]=.23f;v[RingAttack]=.003f;v[RingDecay]=1.7f;v[AfterVCO]=v[AfterVCF]=v[InitialBend]=0;v[Mix]=.28f;break;
 case 5:
  a[LPF]=.42f;a[IL]=.65f;a[AL]=.54f;a[FilterAttack]=.76f;a[AmpAttack]=.67f;a[AmpRelease]=.79f;
  b=a;b[LPF]=.57f;b[HPF]=.08f;b[IL]=.55f;b[AL]=.3f;b[AmpAttack]=.95f;b[FilterAttack]=.91f;b[AmpLevel]=.5f;
  v[FeetII]=3;v[Detune]=3;v[AfterVCO]=.18f;v[AfterVCF]=.25f;v[EffectOn]=1;v[EffectDepth]=.14f;v[EffectSpeed]=.24f;v[Mix]=.34f;v[InitialBend]=.025f;break;
 case 6:
  a[Square]=1;a[Saw]=0;a[PW]=.7f;a[PWMDepth]=.13f;a[LPF]=.38f;a[IL]=.25f;a[AL]=.35f;a[AmpAttack]=.12f;a[AmpSustain]=.8f;a[AmpRelease]=.52f;a[Sine]=.16f;a[AfterBrilliance]=.55f;
  b=a;b[Square]=0;b[Saw]=1;b[LPF]=.32f;b[AmpLevel]=.3f;
  v[Mix]=.22f;v[Detune]=2.5f;v[InitialBend]=.095f;v[AfterVCO]=.25f;v[AfterVCF]=.14f;v[SubSpeed]=5.4f;break;
 case 7:
  a[Square]=1;a[Saw]=0;a[PW]=.56f;a[PWMDepth]=.15f;a[LPF]=.26f;a[IL]=.18f;a[AL]=.2f;a[Sine]=.65f;a[AmpAttack]=.62f;a[AmpRelease]=.82f;a[AmpSustain]=.9f;a[AmpLevel]=.68f;
  b=a;b[Saw]=1;b[Square]=0;b[Sine]=.1f;b[LPF]=.38f;b[AmpAttack]=.8f;b[AmpLevel]=.4f;
  v[FeetI]=0;v[FeetII]=1;v[Detune]=-3;v[Mix]=.3f;v[AfterVCO]=.06f;v[InitialBend]=0;break;
 }
 for(int p=0;p<PatchCount;p++){v[panel(0,p)]=a[p];v[panel(1,p)]=b[p];}
 return sound;
}
}
