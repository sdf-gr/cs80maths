#pragma once
#include "Parameters.hpp"
namespace cs80 {
// Original performances inspired by cinematic CS-80 textures; not transcriptions
// of commercial recordings or claims about an artist's original settings.
constexpr int LegacyPerformancePresetCount=8;
constexpr int PerformancePresetCount=38;
inline const std::array<const char*,PerformancePresetCount> performanceNames={
 "Horizon Brass","Neon Rain","Aegean Strings","Distant Choir",
 "Glass Bells","Solar Flare","Velvet Lead","Low Horizon",
 "Bronze Sky","Rainlit Lead","Starship Swell","Oceanic Choir","Spiral Pulse","Antarctic Pad","Dusk Fanfare","Celestial Bells","Night Harbor Bass","Silver Ribbon","Warmth of Earth","Horizon Solo","Cosmic Fifths","Metallic Orbit","Gentle Memory","Dawn Orchestra","Classic String I","Classic String II","Classic Brass I","Classic Brass II","Breathy Flute","Velvet Electric Piano","Percussive Clav","Baroque Harpsichord","Cathedral Organ","Plucked Guitar","Resonant Funk","Rounded Bass","Tremolo Keys","Reed and Strings"};
inline const std::array<const char*,PerformancePresetCount> performanceNotes={
 "Expressive two-layer brass. Lean into pad pressure to open the tone and add vibrato.",
 "Dark, spacious synth pad with slow PWM and a brighter pressure-controlled upper layer.",
 "Broad, gently detuned strings. Hold chords and use independent pad pressure for movement.",
 "A soft, vocal-like bandpass pad with an octave layer and a slow stereo movement.",
 "Long-decaying sine bells with a shared ring carrier. Release all notes to retrigger its sweep.",
 "A bright cinematic swell: a warm foundation and a slower, pressure-sensitive upper voice.",
 "A rounded singing lead. Initial pitchbend and pressure vibrato supply the articulation.",
 "A deep, restrained bass pad with a sine foundation and a gently moving upper layer.",
 "Warm cinematic brass; pressure brings out the brighter upper layer.",
 "Narrow pulse lead with a soft saw underneath; add pressure for vibrato.",
 "Slow octave brass bloom for wide held chords.",
 "Soft fifths with slowly moving vowel-like filtering.",
 "Rhythmic square-wave filter motion with pressure-controlled pitch movement.",
 "Airy slow strings with a little filtered noise and long release.",
 "Bright, quick brass with a softer octave foundation.",
 "Soft tuned bells with a restrained shared ring carrier.",
 "Deep rounded pulse bass with a dark delayed upper bloom.",
 "Singing octave lead for expressive ribbon bends.",
 "Low warm strings that open under pressure.",
 "Focused brass lead with a strong initial pitch scoop.",
 "Broad fifth-layer strings with gentle stereo movement.",
 "Ring-modulated pluck whose metallic edge falls after each new phrase.",
 "Quiet rounded sine and pulse pad for understated chords.",
 "Layered string-and-brass swell, with brightness responding to pressure.",
 "Classic-style pulse strings with slow PWM and a restrained second layer.",
 "Brighter saw-and-pulse ensemble with a faster onset.",
 "Rounded classic-style saw brass with a clear filter attack.",
 "Brighter layered brass with more pulse edge and pressure response.",
 "Soft flute-like pulse tone with a trace of breath noise.",
 "Velocity-sensitive mellow keys with a sine-rich decay.",
 "Dry narrow-pulse keyboard with short release and touch-sensitive bite.",
 "Bright plucked keyboard with octave reinforcement and little sustain.",
 "Steady pulse and sine registration with octave weight.",
 "A short pulse pluck with a warm sine body.",
 "Snappy high-pass/low-pass pulse bite for rhythmic playing.",
 "Short, low pulse bass with sine support and a gentle filter fall.",
 "Mellow electric keys with the shared stereo tremolo engaged.",
 "A reed-like pulse over a softer slow string layer."};
struct PerformancePreset {std::array<float,ParameterCount>values;};
inline PerformancePreset additionalPerformancePreset(int index);
inline PerformancePreset performancePreset(int index){
 if(index>=LegacyPerformancePresetCount)return additionalPerformancePreset(std::min(index,PerformancePresetCount-1));
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

inline PerformancePreset additionalPerformancePreset(int index){
 const int bases[]={0,6,5,3,0,1,0,4,7,6,2,0,2,4,1,5,100,100,102,102,103,104,105,106,107,108,109,110,104,100};
 int n=std::clamp(index-8,0,29),base=bases[n];auto sound=performancePreset(base>=100?0:base);auto&v=sound.values;
 if(base>=100){int tone=0;switch(base){case 100:tone=0;break;case 102:tone=2;break;case 103:tone=3;break;case 104:tone=4;break;case 105:tone=5;break;case 106:tone=6;break;case 107:tone=7;break;case 108:tone=8;break;case 109:tone=9;break;case 110:tone=2;break;}for(int bank=0;bank<2;bank++){auto p=base==110?preset(1,4):preset(bank,tone);for(int j=0;j<PatchCount;j++)v[panel(bank,j)]=p[j];}}
 switch(n){
 case 0:
  v[Mix]=0.39f;v[Detune]=7;v[InitialBend]=0.07f;
  v[panel(0,LPF)]=0.39f;v[panel(0,FilterAttack)]=0.48f;v[panel(0,AmpAttack)]=0.4f;v[panel(0,AfterBrilliance)]=0.66f;
  v[panel(1,LPF)]=0.52f;v[panel(1,AmpAttack)]=0.57f;v[panel(1,AmpLevel)]=0.46f;
  break;
 case 1:
  v[Mix]=0.32f;v[Detune]=-4;v[SubSpeed]=5.8f;v[AfterVCO]=0.32f;
  v[panel(0,PW)]=0.81f;v[panel(0,LPF)]=0.46f;v[panel(0,PWMDepth)]=0.22f;v[panel(0,AmpRelease)]=0.38f;
  v[panel(1,LPF)]=0.4f;v[panel(1,HPF)]=0.13f;v[panel(1,AmpLevel)]=0.4f;
  break;
 case 2:
  v[FeetII]=3;v[Mix]=0.42f;v[Detune]=8;v[EffectDepth]=0.27f;
  v[panel(0,AmpAttack)]=0.83f;v[panel(0,FilterAttack)]=0.9f;v[panel(0,AL)]=0.7f;v[panel(0,AmpRelease)]=0.9f;
  v[panel(1,AmpAttack)]=1;v[panel(1,LPF)]=0.51f;v[panel(1,AfterBrilliance)]=0.72f;
  break;
 case 3:
  v[FeetII]=2;v[Mix]=0.43f;v[SubSpeed]=0.7f;v[SubVCF]=0.055f;v[EffectDepth]=0.35f;
  v[panel(0,HPF)]=0.19f;v[panel(0,LPF)]=0.42f;v[panel(0,ResH)]=0.38f;v[panel(0,ResL)]=0.48f;
  v[panel(1,HPF)]=0.33f;v[panel(1,LPF)]=0.48f;v[panel(1,ResH)]=0.4f;v[panel(1,AmpLevel)]=0.4f;
  break;
 case 4:
  v[SubFunction]=3;v[SubSpeed]=4.2f;v[SubVCF]=0.17f;v[AfterVCO]=0.12f;v[InitialBend]=0;
  v[panel(0,Saw)]=0;v[panel(0,Square)]=1;v[panel(0,PW)]=0.77f;v[panel(0,LPF)]=0.4f;v[panel(0,ResL)]=0.47f;v[panel(0,AmpAttack)]=0.06f;v[panel(0,IL)]=0;v[panel(0,AL)]=0.12f;
  v[panel(1,Saw)]=0;v[panel(1,Square)]=1;v[panel(1,PW)]=0.55f;v[panel(1,LPF)]=0.32f;v[panel(1,ResL)]=0.24f;v[panel(1,AmpAttack)]=0.08f;
  break;
 case 5:
  v[Detune]=9;v[EffectDepth]=0.38f;v[Mix]=0.46f;
  v[panel(0,HPF)]=0.17f;v[panel(0,Noise)]=0.045f;v[panel(0,LPF)]=0.49f;v[panel(0,AmpAttack)]=0.96f;v[panel(0,AmpRelease)]=0.98f;
  v[panel(1,HPF)]=0.24f;v[panel(1,LPF)]=0.55f;v[panel(1,Noise)]=0.025f;v[panel(1,AmpLevel)]=0.38f;
  break;
 case 6:
  v[FeetII]=0;v[Mix]=0.3f;v[InitialBend]=0.09f;v[Detune]=2;
  v[panel(0,LPF)]=0.58f;v[panel(0,AmpAttack)]=0.12f;v[panel(0,FilterAttack)]=0.19f;v[panel(0,AL)]=0.78f;v[panel(0,AmpRelease)]=0.4f;
  v[panel(1,LPF)]=0.33f;v[panel(1,Square)]=0;v[panel(1,AmpAttack)]=0.25f;v[panel(1,AmpLevel)]=0.45f;
  break;
 case 7:
  v[FeetII]=2;v[RingSpeed]=329.63f;v[RingMod]=0.28f;v[RingDepth]=0.07f;v[RingDecay]=2.8f;v[Mix]=0.24f;
  v[panel(0,AmpAttack)]=0.08f;v[panel(0,AmpDecay)]=0.97f;v[panel(0,AmpRelease)]=0.95f;v[panel(0,Sine)]=0.9f;
  v[panel(1,Sine)]=0.45f;v[panel(1,FilterLevel)]=0.06f;v[panel(1,AmpDecay)]=0.85f;
  break;
 case 8:
  v[Mix]=0.23f;v[Detune]=-1;v[AfterVCF]=0.22f;
  v[panel(0,LPF)]=0.22f;v[panel(0,AmpAttack)]=0.04f;v[panel(0,AmpDecay)]=0.65f;v[panel(0,AmpSustain)]=0.5f;v[panel(0,AmpRelease)]=0.43f;v[panel(0,Sine)]=0.75f;
  v[panel(1,LPF)]=0.36f;v[panel(1,AmpAttack)]=0.62f;v[panel(1,AmpLevel)]=0.3f;
  break;
 case 9:
  v[FeetII]=3;v[Mix]=0.19f;v[AfterVCO]=0.3f;v[InitialBend]=0.13f;
  v[panel(0,PW)]=0.63f;v[panel(0,LPF)]=0.49f;v[panel(0,Sine)]=0.22f;v[panel(0,AmpAttack)]=0.19f;v[panel(0,AmpRelease)]=0.65f;
  v[panel(1,LPF)]=0.43f;v[panel(1,AmpLevel)]=0.31f;v[panel(1,AmpRelease)]=0.73f;
  break;
 case 10:
  v[FeetI]=0;v[FeetII]=1;v[Mix]=0.45f;v[Detune]=4;v[AfterVCF]=0.23f;
  v[panel(0,LPF)]=0.36f;v[panel(0,HPF)]=0;v[panel(0,PW)]=0.71f;v[panel(0,AmpAttack)]=0.88f;
  v[panel(1,LPF)]=0.44f;v[panel(1,AmpAttack)]=0.71f;v[panel(1,AmpLevel)]=0.46f;
  break;
 case 11:
  v[Mix]=0.18f;v[Detune]=1.5f;v[InitialBend]=0.16f;v[AfterVCO]=0.27f;
  v[panel(0,LPF)]=0.5f;v[panel(0,FilterAttack)]=0.22f;v[panel(0,AmpAttack)]=0.13f;v[panel(0,AmpRelease)]=0.48f;v[panel(0,AfterBrilliance)]=0.7f;
  v[panel(1,LPF)]=0.36f;v[panel(1,AmpLevel)]=0.35f;v[panel(1,AmpAttack)]=0.3f;
  break;
 case 12:
  v[FeetII]=2;v[Mix]=0.36f;v[Detune]=-6;v[EffectDepth]=0.36f;v[EffectSpeed]=0.11f;
  v[panel(0,LPF)]=0.48f;v[panel(0,PW)]=0.67f;v[panel(0,PWMDepth)]=0.36f;v[panel(0,AmpRelease)]=0.94f;
  v[panel(1,LPF)]=0.56f;v[panel(1,AmpAttack)]=0.92f;v[panel(1,AmpLevel)]=0.4f;
  break;
 case 13:
  v[FeetII]=1;v[Mix]=0.37f;v[RingSpeed]=147;v[RingMod]=0.76f;v[RingDepth]=0.45f;v[RingAttack]=0.015f;v[RingDecay]=0.7f;
  v[panel(0,AmpDecay)]=0.66f;v[panel(0,AmpRelease)]=0.63f;v[panel(0,Sine)]=0.78f;
  v[panel(1,Sine)]=0.5f;v[panel(1,FilterLevel)]=0.19f;v[panel(1,AmpDecay)]=0.8f;
  break;
 case 14:
  v[EffectOn]=0;v[Mix]=0.3f;v[AfterVCO]=0.06f;v[Detune]=2;
  v[panel(0,Sine)]=0.55f;v[panel(0,FilterLevel)]=0.35f;v[panel(0,LPF)]=0.29f;v[panel(0,AmpAttack)]=0.7f;v[panel(0,AmpRelease)]=0.85f;
  v[panel(1,LPF)]=0.37f;v[panel(1,AmpLevel)]=0.32f;v[panel(1,AmpAttack)]=0.83f;v[panel(1,Sine)]=0.15f;
  break;
 case 15:
  v[FeetII]=1;v[Mix]=0.47f;v[Detune]=6;v[EffectDepth]=0.19f;
  v[panel(0,Square)]=1;v[panel(0,Saw)]=0;v[panel(0,PWMDepth)]=0.5f;v[panel(0,PW)]=0.66f;v[panel(0,LPF)]=0.5f;v[panel(0,IL)]=0.2f;v[panel(0,AL)]=0.18f;v[panel(0,AmpAttack)]=0.82f;
  v[panel(1,LPF)]=0.47f;v[panel(1,AL)]=0.65f;v[panel(1,FilterAttack)]=0.72f;v[panel(1,AmpAttack)]=0.65f;v[panel(1,AmpLevel)]=0.53f;
  break;
 case 16:
  v[Detune]=4;v[Mix]=0.34f;v[EffectOn]=1;v[EffectDepth]=0.16f;v[InitialBend]=0;
  v[panel(0,PW)]=0.63f;v[panel(0,LPF)]=0.56f;v[panel(0,AmpLevel)]=0.65f;
  v[panel(1,LPF)]=0.48f;v[panel(1,AmpLevel)]=0.48f;v[panel(1,AmpAttack)]=0.72f;
  break;
 case 17:
  v[Mix]=0.47f;v[Detune]=8;v[EffectOn]=1;v[EffectDepth]=0.28f;v[InitialBend]=0;
  v[panel(0,Saw)]=1;v[panel(0,Square)]=0;v[panel(0,LPF)]=0.61f;v[panel(0,AmpAttack)]=0.48f;v[panel(0,AmpLevel)]=0.57f;
  v[panel(1,PW)]=0.72f;v[panel(1,PWMDepth)]=0.6f;v[panel(1,LPF)]=0.53f;v[panel(1,AmpAttack)]=0.65f;v[panel(1,AmpLevel)]=0.44f;
  break;
 case 18:
  v[Mix]=0.37f;v[InitialBend]=0.025f;v[Detune]=3;
  v[panel(0,LPF)]=0.39f;v[panel(0,AL)]=0.7f;v[panel(0,IL)]=0.62f;v[panel(0,AmpAttack)]=0.16f;v[panel(0,AmpLevel)]=0.68f;
  v[panel(1,LPF)]=0.46f;v[panel(1,FilterAttack)]=0.4f;v[panel(1,AmpAttack)]=0.3f;v[panel(1,AmpLevel)]=0.43f;
  break;
 case 19:
  v[Mix]=0.46f;v[InitialBend]=0.045f;v[Detune]=5;
  v[panel(0,LPF)]=0.53f;v[panel(0,ResL)]=0.28f;v[panel(0,FilterAttack)]=0.2f;v[panel(0,AmpAttack)]=0.1f;v[panel(0,AmpLevel)]=0.6f;
  v[panel(1,Square)]=1;v[panel(1,PW)]=0.59f;v[panel(1,LPF)]=0.44f;v[panel(1,AfterBrilliance)]=0.6f;v[panel(1,AmpLevel)]=0.42f;
  break;
 case 20:
  v[Mix]=0.2f;v[Detune]=1;v[AfterVCO]=0.15f;v[InitialBend]=0;
  v[panel(0,Noise)]=0.035f;v[panel(0,LPF)]=0.3f;v[panel(0,AmpAttack)]=0.38f;v[panel(0,Sine)]=0.25f;v[panel(0,AmpLevel)]=0.72f;
  v[panel(1,LPF)]=0.25f;v[panel(1,Sine)]=0.4f;v[panel(1,AmpLevel)]=0.3f;v[panel(1,AmpAttack)]=0.48f;
  break;
 case 21:
  v[Mix]=0.31f;v[FeetII]=3;v[Detune]=1;v[AfterVCO]=0;v[InitialBend]=0;
  v[panel(0,Sine)]=0.48f;v[panel(0,LPF)]=0.33f;v[panel(0,AmpDecay)]=0.72f;v[panel(0,AmpSustain)]=0.08f;v[panel(0,AmpLevel)]=0.65f;
  v[panel(1,Sine)]=0.65f;v[panel(1,FilterLevel)]=0.32f;v[panel(1,LPF)]=0.25f;v[panel(1,AmpDecay)]=0.57f;v[panel(1,AmpLevel)]=0.35f;
  break;
 case 22:
  v[Mix]=0.25f;v[Detune]=-2;v[AfterVCO]=0;v[InitialBend]=0;
  v[panel(0,PW)]=0.84f;v[panel(0,LPF)]=0.57f;v[panel(0,HPF)]=0.22f;v[panel(0,AmpDecay)]=0.46f;v[panel(0,AmpRelease)]=0.16f;v[panel(0,AmpLevel)]=0.64f;
  v[panel(1,PW)]=0.73f;v[panel(1,LPF)]=0.43f;v[panel(1,AmpDecay)]=0.38f;v[panel(1,AmpLevel)]=0.35f;
  break;
 case 23:
  v[FeetII]=3;v[Mix]=0.24f;v[AfterVCO]=0;v[InitialBend]=0;
  v[panel(0,LPF)]=0.65f;v[panel(0,HPF)]=0.18f;v[panel(0,AmpDecay)]=0.58f;v[panel(0,AmpRelease)]=0.22f;v[panel(0,AmpLevel)]=0.6f;
  v[panel(1,PW)]=0.8f;v[panel(1,LPF)]=0.5f;v[panel(1,AmpDecay)]=0.47f;v[panel(1,AmpLevel)]=0.36f;
  break;
 case 24:
  v[FeetII]=0;v[Mix]=0.42f;v[Detune]=0;v[InitialBend]=0;v[AfterVCO]=0;v[AfterVCF]=0;
  v[panel(0,Sine)]=0.35f;v[panel(0,LPF)]=0.75f;v[panel(0,AmpLevel)]=0.55f;v[panel(0,AmpRelease)]=0.16f;
  v[panel(1,PW)]=0.68f;v[panel(1,Sine)]=0.6f;v[panel(1,LPF)]=0.62f;v[panel(1,AmpLevel)]=0.42f;v[panel(1,AmpRelease)]=0.2f;
  break;
 case 25:
  v[Mix]=0.28f;v[Detune]=2;v[InitialBend]=0;v[AfterVCO]=0.04f;
  v[panel(0,Sine)]=0.18f;v[panel(0,LPF)]=0.4f;v[panel(0,AmpDecay)]=0.6f;v[panel(0,AmpRelease)]=0.38f;v[panel(0,AmpLevel)]=0.65f;
  v[panel(1,PW)]=0.68f;v[panel(1,LPF)]=0.32f;v[panel(1,AmpDecay)]=0.47f;v[panel(1,AmpLevel)]=0.36f;
  break;
 case 26:
  v[Mix]=0.32f;v[InitialBend]=0;v[AfterVCF]=0.26f;
  v[panel(0,HPF)]=0.32f;v[panel(0,LPF)]=0.49f;v[panel(0,ResH)]=0.42f;v[panel(0,ResL)]=0.48f;v[panel(0,FilterDecay)]=0.34f;v[panel(0,AmpLevel)]=0.52f;
  v[panel(1,HPF)]=0.2f;v[panel(1,LPF)]=0.38f;v[panel(1,ResH)]=0.2f;v[panel(1,ResL)]=0.38f;v[panel(1,AmpLevel)]=0.32f;
  break;
 case 27:
  v[FeetI]=0;v[FeetII]=0;v[Mix]=0.27f;v[InitialBend]=0.025f;v[AfterVCO]=0;
  v[panel(0,Square)]=1;v[panel(0,Saw)]=0;v[panel(0,PW)]=0.56f;v[panel(0,LPF)]=0.24f;v[panel(0,Sine)]=0.4f;v[panel(0,AmpDecay)]=0.54f;v[panel(0,AmpLevel)]=0.6f;
  v[panel(1,LPF)]=0.3f;v[panel(1,Sine)]=0.25f;v[panel(1,AmpSustain)]=0.2f;v[panel(1,AmpLevel)]=0.34f;
  break;
 case 28:
  v[Mix]=0.39f;v[EffectOn]=1;v[Tremolo]=1;v[EffectSpeed]=0.18f;v[EffectDepth]=0.47f;v[InitialBend]=0;v[AfterVCO]=0;
  v[panel(0,Sine)]=0.3f;v[panel(0,LPF)]=0.37f;v[panel(0,AmpDecay)]=0.78f;v[panel(0,AmpSustain)]=0.18f;v[panel(0,AmpLevel)]=0.58f;
  v[panel(1,Sine)]=0.45f;v[panel(1,LPF)]=0.31f;v[panel(1,AmpDecay)]=0.65f;v[panel(1,AmpLevel)]=0.38f;
  break;
 case 29:
  v[Mix]=0.38f;v[Detune]=3;v[InitialBend]=0.025f;
  v[panel(0,PW)]=0.78f;v[panel(0,PWMDepth)]=0.06f;v[panel(0,LPF)]=0.42f;v[panel(0,HPF)]=0.13f;v[panel(0,AmpAttack)]=0.23f;v[panel(0,AmpLevel)]=0.65f;
  v[panel(1,PW)]=0.61f;v[panel(1,PWMDepth)]=0.46f;v[panel(1,LPF)]=0.5f;v[panel(1,AmpAttack)]=0.81f;v[panel(1,AmpRelease)]=0.8f;v[panel(1,AmpLevel)]=0.45f;
  break;
 }
 return sound;
}
}
