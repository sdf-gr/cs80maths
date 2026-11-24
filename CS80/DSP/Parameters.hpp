#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
namespace cs80 {
enum Global { Volume, Mix, FeetI, FeetII, Detune, Coarse, Fine, Brilliance, Resonance,
 LowBrilliance, HighBrilliance, LowLevel, HighLevel, SubFunction, SubSpeed, SubVCO, SubVCF, SubVCA,
 InitialBend, AfterSpeed, AfterVCO, AfterVCF, Sustain, SustainMode, SustainAssign, Glide, Glissando,
 GlideAssign, EffectOn, Tremolo, EffectSpeed, EffectDepth, RingSpeed, RingMod, RingAttack, RingDecay,
 RingDepth, Expression, ExpressionWah, FootSwitch, PedalConnected, Ribbon, MPE, Variation,
 OutputLow, ExternalLevel, ToneI, ToneII, GlobalCount };
enum Patch { PWMSpeed, PWMDepth, PW, Square, Saw, Noise, HPF, ResH, LPF, ResL, IL, AL,
 FilterAttack, FilterDecay, FilterRelease, FilterLevel, Sine, AmpAttack, AmpDecay, AmpSustain,
 AmpRelease, AmpLevel, InitialBrilliance, InitialLevel, AfterBrilliance, AfterLevel, PatchCount };
constexpr int PanelBase=GlobalCount, MemoryBase=PanelBase+2*PatchCount, ParameterCount=MemoryBase+4*PatchCount;
constexpr int panel(int bank,int p) {return PanelBase+bank*PatchCount+p;}
constexpr int memory(int slot,int p) {return MemoryBase+slot*PatchCount+p;}
struct Spec { std::string id,name,group; float lo=0,hi=1,initial=0; bool discrete=false; std::vector<std::string> choices; };
inline const std::array<const char*,PatchCount> patchNames={"PWM speed","PWM depth","Pulse width","Pulse wave","Sawtooth wave","White noise","HPF cutoff","HPF resonance","LPF cutoff","LPF resonance","Initial level (IL)","Attack level (AL)","Filter attack","Filter decay","Filter release","Filtered signal level","Sine wave level","Amplitude attack","Amplitude decay","Amplitude sustain","Amplitude release","Voice level","Initial brilliance","Initial level","After brilliance","After level"};
using PatchValues=std::array<float,PatchCount>;
// Factory voices are manual-informed starting points, not measured resistor-matrix voltages.
inline PatchValues preset(int bank,int index) {
 PatchValues p={.17f,0,.5f,0,1,0,0,0,.66f,.1f,.55f,.65f,.2f,.36f,.3f,1,0,.08f,.45f,.75f,.3f,.75f,.15f,.45f,.25f,.25f};
 int kind=0;
 if(bank==0) {const int k[]={0,0,1,2,3,4,5,6,7,8,8}; kind=k[std::clamp(index,0,10)];}
 else {const int k[]={0,0,1,1,9,4,5,6,7,8,8};kind=k[std::clamp(index,0,10)];}
 switch(kind) {
 case 0:p[Square]=1;p[Saw]=0;p[PW]=.65f;p[PWMDepth]=.45f;p[PWMSpeed]=.24f;p[LPF]=.58f;p[IL]=.25f;p[AL]=.12f;p[AmpAttack]=.55f;p[FilterAttack]=.5f;p[AmpSustain]=.85f;p[AmpRelease]=.6f;break;
 case 1:p[LPF]=.42f;p[IL]=.7f;p[AL]=.85f;p[FilterAttack]=.3f;p[FilterDecay]=.45f;p[ResL]=.2f;break;
 case 2:p[LPF]=.32f;p[IL]=p[AL]=0;p[AmpAttack]=.3f;p[AmpSustain]=1;p[Sine]=.15f;break;
 case 3:p[Square]=1;p[Saw]=0;p[PW]=.62f;p[LPF]=.36f;p[IL]=p[AL]=0;p[AmpAttack]=0;p[AmpDecay]=.62f;p[AmpSustain]=.05f;p[InitialLevel]=.8f;p[AfterLevel]=0;break;
 case 4:case 5:p[Square]=1;p[Saw]=0;p[PW]=.9f;p[HPF]=.22f;p[LPF]=.5f;p[IL]=0;p[AL]=.45f;p[FilterAttack]=0;p[FilterDecay]=.32f;p[AmpAttack]=0;p[AmpDecay]=.46f;p[AmpSustain]=0;p[AmpRelease]=.12f;p[AfterLevel]=0;break;
 case 6:p[Square]=1;p[Saw]=0;p[PW]=.5f;p[Sine]=.4f;p[IL]=p[AL]=0;p[LPF]=.85f;p[AmpAttack]=0;p[AmpSustain]=1;p[AmpRelease]=.05f;p[InitialLevel]=p[AfterLevel]=0;break;
 case 7:p[Square]=1;p[Saw]=0;p[PW]=.85f;p[HPF]=.2f;p[LPF]=.45f;p[AL]=.45f;p[IL]=0;p[FilterAttack]=0;p[AmpAttack]=0;p[AmpDecay]=.5f;p[AmpSustain]=0;p[AmpRelease]=.28f;break;
 case 8:p[Square]=1;p[Saw]=0;p[PW]=.75f;p[HPF]=.4f;p[LPF]=.55f;p[ResH]=.5f;p[ResL]=.6f;p[IL]=.7f;p[AL]=.85f;p[FilterAttack]=0;p[FilterDecay]=.4f;p[AmpAttack]=0;break;
 case 9:p[LPF]=.18f;p[IL]=.4f;p[AL]=.6f;p[AmpAttack]=0;p[AmpDecay]=.6f;p[AmpSustain]=.35f;p[Sine]=.5f;break;
 }
 if(index==1){p[LPF]*=.88f;p[PWMDepth]=std::min(1.f,p[PWMDepth]+.15f);}
 return p;
}
inline std::vector<Spec> makeSpecs() {
 std::vector<Spec>s;
 auto add=[&](const char*id,const char*name,const char*group,float lo,float hi,float v,bool d=false,std::vector<std::string> c={}) {s.push_back({id,name,group,lo,hi,v,d,c});};
 add("volume","Master volume","Output",0,1,.65);add("mix","Mix I / II","Tuning",0,1,.5);
 std::vector<std::string> feet={"16′","8′","5⅓′","4′","2⅔′","2′"};
 add("feetI","Feet I","Tuning",0,5,1,true,feet);add("feetII","Feet II","Tuning",0,5,1,true,feet);
 add("detune","Detune II (cents)","Tuning",-100,100,3);add("coarse","Coarse pitch (cents)","Tuning",-500,700,0);add("fine","Fine pitch (cents)","Tuning",-50,50,0);
 add("brilliance","Brilliance","Keyboard",-1,1,0);add("resonance","Resonance","Keyboard",0,1,0);
 add("lowBrilliance","Low brilliance","Keyboard",-1,1,0);add("highBrilliance","High brilliance","Keyboard",-1,1,0);add("lowLevel","Low level","Keyboard",-1,1,0);add("highLevel","High level","Keyboard",-1,1,0);
 add("subFunction","Function","Sub oscillator",0,5,0,true,{"Sine","Saw down","Saw up","Square","Noise","External"});
 add("subSpeed","Speed (Hz)","Sub oscillator",.7,60,5);add("subVCO","VCO depth","Sub oscillator",0,1,0);add("subVCF","VCF depth","Sub oscillator",0,1,0);add("subVCA","VCA depth","Sub oscillator",0,1,0);
 add("initialBend","Initial pitchbend","Touch response",0,1,0);add("afterSpeed","After speed","Touch response",0,1,0);add("afterVCO","After VCO","Touch response",0,1,.12);add("afterVCF","After VCF","Touch response",0,1,.1);
 add("sustain","Sustain (seconds)","Sustain & glide",0,10,0);add("sustainMode","Sustain mode","Sustain & glide",0,1,0,true,{"I — independent","II — last phrase"});add("sustainAssign","Sustain follows foot switch","Sustain & glide",0,1,0,true);
 add("glide","Full-range glide (seconds)","Sustain & glide",0,10,0);add("glissando","Pitch transition","Sustain & glide",0,1,0,true,{"Portamento","Glissando"});add("glideAssign","Glide follows foot switch","Sustain & glide",0,1,0,true);
 add("effectOn","Effect on","Tremolo / chorus",0,1,0,true);add("tremolo","Effect mode","Tremolo / chorus",0,1,0,true,{"Chorus","Tremolo"});add("effectSpeed","Speed","Tremolo / chorus",0,1,.3);add("effectDepth","Depth","Tremolo / chorus",0,1,.4);
 add("ringSpeed","Carrier speed (Hz)","Ring modulator",.5,2000,100);add("ringMod","Modulation","Ring modulator",0,1,0);add("ringAttack","Attack (seconds)","Ring modulator",.001,5,.02);add("ringDecay","Decay (seconds)","Ring modulator",.01,10,.8);add("ringDepth","Frequency envelope depth","Ring modulator",0,1,0);
 add("expression","Expression pedal","Output",0,1,1);add("expressionWah","Pedal function","Output",0,1,0,true,{"Expression","Expression + wah"});add("footSwitch","Foot switch down","Sustain & glide",0,1,0,true);add("pedalConnected","Foot switch connected","Sustain & glide",0,1,0,true);
 add("ribbon","Ribbon displacement","Ribbon",-1,1,0);add("mpe","MPE input","Input & modeling",0,1,0,true);add("variation","Voice variation","Input & modeling",0,1,.25);add("outputLow","Output range","Output",0,1,0,true,{"High","Low (−20 dB)"});add("externalLevel","External modulation gain","Input & modeling",0,4,1);
 std::vector<std::string>a={"String 1","String 3","Brass 1","Flute","Electric piano","Clavichord 1","Harpsichord 1","Organ 1","Guitar 1","Funky 1","Funky 3","Memory 1","Memory 3","Panel"};
 std::vector<std::string>b={"String 2","String 4","Brass 2","Brass 3","Bass","Clavichord 2","Harpsichord 2","Organ 2","Guitar 2","Funky 2","Funky 4","Memory 2","Memory 4","Panel"};
 add("toneI","Tone I","Tone selectors",0,13,2,true,a);add("toneII","Tone II","Tone selectors",0,13,2,true,b);
 for(int block=0;block<6;block++) {std::string g=block<2?(block==0?"Channel I panel":"Channel II panel"):"Memory "+std::to_string(block-1);auto v=preset(block%2,2);
  for(int p=0;p<PatchCount;p++){std::string id="p"+std::to_string(block)+"_"+std::to_string(p);float lo=0,hi=1,def=v[p];if(p==PW){lo=.5f;hi=.9f;}bool d=(p==Square||p==Saw);s.push_back({id,patchNames[p],g,lo,hi,def,d,{}});}}
 return s;
}
inline float timeValue(float x,bool attack=false){return (attack?.001f:.01f)*std::pow(1000.f,std::clamp(x,0.f,1.f));}
}
