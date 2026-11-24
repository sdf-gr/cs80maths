#include "Engine.hpp"
#include <limits>
namespace cs80 {
constexpr double pi=3.14159265358979323846;
static double soft(double x){return std::tanh(x);}
static double blep(double t,double dt){if(t<dt){t/=dt;return 2*t-t*t-1;}if(t>1-dt){t=(t-1)/dt;return t*t+2*t+1;}return 0;}
Engine::Engine():specs(makeSpecs()){for(int i=0;i<ParameterCount;i++){published[i].store(specs[i].initial);values[i]=seen[i]=specs[i].initial;}prepare(48000);}
void Engine::prepare(double sr){sampleRate=std::clamp(sr,8000.,384000.);pressureSlew=1-std::exp(-1/(sampleRate*2*.002));bendSlew=1-std::exp(-1/(sampleRate*.002));double sum=0;for(int i=0;i<63;i++){double x=i-31;fir[i]=(x==0?.45:std::sin(pi*.45*x)/(pi*x))*(.54-.46*std::cos(2*pi*i/62));sum+=fir[i];}for(auto&x:fir)x/=sum;reset();}
void Engine::reset(){voices={};retiredVoices={};pressure={};bend={};smoothBend={};ignored={};nextVoice=controlCounter=0;subPhase=ringPhase=effectPhase=0;pwmPhase={};delay.fill(0);delayIndex=0;downL=downR=dcL=dcR=prevL=prevR=0;ringEnvelope={};effectMix=0;firHistory={};firIndex=0;
 for(int i=0;i<ParameterCount;i++){values[i]=seen[i]=published[i].load();ramps[i]={};}for(int i=0;i<8;i++){voices[i].circuit=i;for(int b=0;b<2;b++)voices[i].layers[b].phase=std::fmod(i*.127+b*.317,1.);}updatePatch();}
void Engine::set(int a,float v){if(a<0||a>=ParameterCount||!std::isfinite(v))return;v=std::clamp(v,specs[a].lo,specs[a].hi);if(specs[a].discrete)v=std::round(v);published[a].store(v,std::memory_order_relaxed);}
float Engine::get(int a)const{return a>=0&&a<ParameterCount?published[a].load(std::memory_order_relaxed):0;}
void Engine::eventParameter(int a,float v,uint32_t frames){if(a<0||a>=ParameterCount||!std::isfinite(v))return;set(a,v);v=get(a);seen[a]=v;if(!frames||specs[a].discrete){values[a]=v;ramps[a]={};}else{ramps[a]={ (v-values[a])/frames,frames };}controlCounter=0;}
void Engine::tickParameters(){for(int a=0;a<ParameterCount;a++){float v=published[a].load(std::memory_order_relaxed);if(v!=seen[a]){seen[a]=v;if(specs[a].discrete){values[a]=v;ramps[a]={};}else{uint32_t n=uint32_t(sampleRate*.005);ramps[a]={(v-values[a])/n,n};}}
 if(ramps[a].left){values[a]+=ramps[a].step;if(!--ramps[a].left)values[a]=seen[a];}}}
bool Engine::pedal()const{return values[PedalConnected]<.5f||values[FootSwitch]>.5f;}
bool Engine::sustainOn()const{return values[SustainAssign]<.5f||pedal();}
bool Engine::glideOn()const{return values[Glide]>.0001f&&(values[GlideAssign]<.5f||pedal());}
int Engine::heldCount()const{int n=0;for(auto&v:voices)n+=v.held;return n;}
int Engine::activeCount()const{int n=0;for(auto&v:voices)n+=v.active;return n;}
float Engine::random(){randomState^=randomState<<13;randomState^=randomState>>17;randomState^=randomState<<5;return float(double(randomState)/2147483648.-1.);}
void Engine::release(Voice&v){v.held=false;for(int b=0;b<2;b++){auto&l=v.layers[b];l.amp.begin(Envelope::Release,0);l.filter.begin(Envelope::Release,-patch[b][IL]);}}
// Preserve the old waveform for a 0.5 ms transition instead of dropping a live
// release tail to zero. Storage is fixed; MIDI handling never allocates.
void Engine::retire(const Voice&v){
 if(!v.active)return;
 Voice*slot=&retiredVoices[0];
 for(auto&candidate:retiredVoices){if(!candidate.active){slot=&candidate;break;}if(candidate.fadeOutLeft<slot->fadeOutLeft)slot=&candidate;}
 *slot=v;slot->held=false;slot->fadeInLeft=0;
 slot->fadeTotal=slot->fadeOutLeft=std::max(8u,uint32_t(sampleRate*2*.0005));
 slot->fadeStart=v.lastFadeGain;
}
void Engine::noteOn(int note,float vel,int channel){
 updatePatch();
 // Eight held keys take priority; a ninth is ignored, never stolen.
 if(heldCount()==8){ignored[channel][note]++;return;}
 bool first=heldCount()==0;
 if(values[SustainMode]>.5f)for(auto&v:voices)if(v.active&&!v.held){retire(v);v.active=false;for(auto&l:v.layers){l.amp={};l.filter={};}}
 int slot=-1;for(int j=0;j<8;j++){int i=(nextVoice+j)%8;if(!voices[i].held){slot=i;break;}}if(slot<0)return;
 nextVoice=(slot+1)%8;auto&v=voices[slot];retire(v);
 // A half-millisecond onset edge also removes the slope discontinuity of
 // a minimum-time envelope starting at an arbitrary free-running phase.
 v.fadeTotal=std::max(8u,uint32_t(sampleRate*2*.0005));v.fadeInLeft=v.fadeTotal;v.fadeOutLeft=0;v.fadeStart=1;v.lastFadeGain=0;
 v.active=v.held=true;v.note=note;v.channel=channel;v.velocity=vel;v.pressure=pressure[channel];v.smoothPressure=0;v.initial=vel;
 v.key=glideOn()?v.key:note;v.target=note;v.ribbon=1;
 for(int b=0;b<2;b++){auto&l=v.layers[b];
  l.amp.trigger(0,1);l.filter.trigger(-patch[b][IL],patch[b][AL]);l.env=-patch[b][IL];
  // The retired copy keeps its filter history. A new note must not resume a
  // resonant filter state that was frozen while this assignment was idle.
  l.hp.s1=l.hp.s2=l.lp.s1=l.lp.s2=0;
 }
 controlCounter=0;
 if(first)ringEnvelope.trigger(0,1);
}
void Engine::noteOff(int note,int channel){if(ignored[channel][note]){ignored[channel][note]--;return;}for(auto&v:voices)if(v.held&&v.note==note&&v.channel==channel){release(v);break;}}
void Engine::midi(uint8_t status,uint8_t d1,uint8_t d2){if(d1>127||d2>127)return;int ch=status&15;
 auto cc=[&](int address,float value){eventParameter(address,value,std::max(1u,uint32_t(sampleRate*.005)));};
 switch(status&0xf0){
 case 0x90:if(d2)noteOn(d1,d2/127.f,ch);else noteOff(d1,ch);break;case 0x80:noteOff(d1,ch);break;
 case 0xa0:for(auto&v:voices)if(v.active&&v.channel==ch&&v.note==d1)v.pressure=d2/127.f;break;
 case 0xd0:pressure[ch]=d1/127.f;for(auto&v:voices)if(v.active&&(v.channel==ch||(values[MPE]<.5f&&ch==0)))v.pressure=pressure[ch];break;
 case 0xe0:bend[ch]=((int(d2)*128+d1)-8192)/8192.f;break;
 case 0xb0:switch(d1){case 1:cc(SubVCO,d2/127.f);break;case 7:cc(Volume,d2/127.f);break;case 11:cc(Expression,d2/127.f);break;
 case 64:eventParameter(PedalConnected,1);eventParameter(FootSwitch,d2>=64?1:0);break;
 case 16:cc(Brilliance,d2/63.5f-1);break;case 17:cc(Resonance,d2/127.f);break;case 18:cc(Mix,d2/127.f);break;case 19:cc(SubVCO,d2/127.f);break;case 20:cc(SubVCF,d2/127.f);break;case 21:cc(RingMod,d2/127.f);break;case 22:cc(RingSpeed,float(.5*std::pow(4000.,d2/127.)));break;case 23:cc(Expression,d2/127.f);break;
 case 74:if(values[MPE]>.5f){/* Timbre mapped to per-note brightness via pressure only when explicitly sent as pressure. */}break;
 case 120:for(auto&v:voices)if(v.channel==ch){v.active=v.held=false;for(auto&l:v.layers){l.amp={};l.filter={};}}for(auto&v:retiredVoices)if(v.channel==ch)v.active=false;ignored[ch]={};break;
 case 121:pressure[ch]=bend[ch]=0;for(auto&v:voices)if(v.channel==ch)v.pressure=0;cc(Expression,1);eventParameter(FootSwitch,0);break;
 case 123:for(auto&v:voices)if(v.held&&v.channel==ch)release(v);ignored[ch]={};break;}
 break;}
}
void Engine::updatePatch(){for(int b=0;b<2;b++){int tone=int(std::round(values[b?ToneII:ToneI]));if(tone<11)patch[b]=preset(b,tone);else{int base=tone==13?panel(b,0):memory((tone==11?0:2)+b,0);for(int p=0;p<PatchCount;p++)patch[b][p]=values[base+p];}}}
void Engine::controls(){updatePatch();for(int index=0;index<24;index++){auto&v=index<8?voices[index]:retiredVoices[index-8];if(!v.active)continue;for(int b=0;b<2;b++){
 auto&p=patch[b];auto&l=v.layers[b];double low=std::max(0.,(60.-v.note)/36.),high=std::max(0.,(v.note-60.)/24.);
 double wah=values[ExpressionWah]>.5f?(values[Expression]-.5)*4:0;
 double shift=values[Brilliance]*3+values[LowBrilliance]*low*2+values[HighBrilliance]*high*2+l.env*4+p[InitialBrilliance]*v.initial*3+p[AfterBrilliance]*v.smoothPressure*3+wah;
 double drift=values[Variation]*std::sin((v.circuit+1)*4.197+b*1.73)*.09;
 double hp=16.35*std::exp2(p[HPF]*10+(v.key-60)/12+shift+drift),lp=16.35*std::exp2(p[LPF]*10+(v.key-60)/12+shift+drift);
 l.hp.tune(hp,std::clamp(double(p[ResH]+values[Resonance]*.7f+(values[ExpressionWah]>.5f?.2f:0.f)),0.,.98),sampleRate*2);
 l.lp.tune(lp,std::clamp(double(p[ResL]+values[Resonance]*.7f+(values[ExpressionWah]>.5f?.2f:0.f)),0.,.98),sampleRate*2);
 }}}
void Engine::process(float*left,float*right,uint32_t count,const float*external){
 static constexpr int feet[]={-12,0,7,12,19,24};
 const double dt=1/sampleRate,osRate=sampleRate*2;
 for(uint32_t n=0;n<count;n++){
  tickParameters();if(controlCounter++%16==0)controls();
  for(int ch=0;ch<16;ch++)smoothBend[ch]+=(bend[ch]-smoothBend[ch])*bendSlew;
  float commonNoise=random(),maxPressure=0;for(auto&v:voices)if(v.held)maxPressure=std::max(maxPressure,v.smoothPressure);
  double subHz=values[SubSpeed]*std::exp2(values[AfterSpeed]*maxPressure*3);subPhase+=std::min(subHz,sampleRate*.4)*dt;subPhase-=std::floor(subPhase);
  double sub=0;switch(int(values[SubFunction])){case 0:sub=std::sin(2*pi*subPhase);break;case 1:sub=1-2*subPhase;break;case 2:sub=2*subPhase-1;break;case 3:sub=subPhase<.5?1:-1;break;case 4:sub=commonNoise;break;case 5:sub=external?external[n]*values[ExternalLevel]:0;break;}
  for(int b=0;b<2;b++){pwmPhase[b]+=(.05*std::pow(1000.,patch[b][PWMSpeed]))*dt;pwmPhase[b]-=std::floor(pwmPhase[b]);pwm[b]=std::sin(2*pi*pwmPhase[b]);}
  double stereo[2]={0,0};
  for(int os=0;os<2;os++){
   double banks[2]={0,0};
   for(int i=0;i<24;i++){auto&v=i<8?voices[i]:retiredVoices[i-8];if(!v.active){if(i>=8)continue;for(int b=0;b<2;b++){double freq=440*std::exp2((v.key+feet[std::clamp(int(values[b?FeetII:FeetI]),0,5)]-69)/12);auto&ph=v.layers[b].phase;ph+=std::clamp(freq/osRate,.00000001,.45);ph-=std::floor(ph);}continue;}
    if(glideOn()&&(v.held||values[SustainMode]>.5f)){double step=60./std::max(.0001f,values[Glide])/osRate;v.key+=std::clamp(v.target-v.key,-step,step);}else if(v.held)v.key=v.target;
    double globalRibbon=std::max(.00005,1.+values[Ribbon]);if(v.held||values[SustainMode]>.5f)v.ribbon=globalRibbon;
    v.smoothPressure+=(v.pressure-v.smoothPressure)*pressureSlew;
    double channelBend=smoothBend[0]*2;if(values[MPE]>.5f&&v.channel!=0)channelBend+=smoothBend[v.channel]*48;else if(v.channel!=0)channelBend=smoothBend[v.channel]*2;
    v.initial*=std::exp(-1./(.12*osRate));if(v.initial<1e-12f)v.initial=0;
    double keysLow=std::max(0.,(60.-v.note)/36.),keysHigh=std::max(0.,(v.note-60.)/24.);
    double fade=1;
    if(v.fadeOutLeft){double t=double(v.fadeTotal-v.fadeOutLeft+1)/v.fadeTotal;fade=v.fadeStart*.5*(1+std::cos(pi*t));if(!--v.fadeOutLeft)v.active=false;}
    else if(v.fadeInLeft){double t=double(v.fadeTotal-v.fadeInLeft+1)/v.fadeTotal;fade=.5*(1-std::cos(pi*t));--v.fadeInLeft;}
    v.lastFadeGain=float(fade);
    for(int b=0;b<2;b++){auto&p=patch[b];auto&l=v.layers[b];
     float releaseTime=timeValue(p[AmpRelease]);if(sustainOn())releaseTime=std::max(releaseTime,values[Sustain]);
     if(!v.held&&!sustainOn()&&values[SustainAssign]>.5f)releaseTime=std::min(releaseTime,.015f);
     l.ampValue=l.amp.tick(1/osRate,timeValue(p[AmpAttack],true),timeValue(p[AmpDecay]),releaseTime,p[AmpSustain],0);
     float fenv=l.filter.tick(1/osRate,timeValue(p[FilterAttack],true),timeValue(p[FilterDecay]),timeValue(p[FilterRelease]),0,-p[IL],p[AL]);
     l.env=fenv+float(sub*(values[SubVCF]+values[AfterVCF]*v.smoothPressure));
     double cents=values[Coarse]+values[Fine]+(b?values[Detune]:0)+values[Variation]*std::sin((v.circuit+1)*2.731+b*4.7)*5;
     double playedKey=values[Glissando]>.5f?std::round(v.key):v.key;
     double pitch=playedKey+feet[std::clamp(int(values[b?FeetII:FeetI]),0,5)]+cents/100+channelBend-values[InitialBend]*v.initial*12;
     pitch+=sub*(values[SubVCO]+values[AfterVCO]*v.smoothPressure)*2;
     double frequency=440*std::exp2((pitch-69)/12)*v.ribbon,increment=std::clamp(frequency/osRate,.00000001,.45);
     l.phase+=increment;l.phase-=std::floor(l.phase);double ph=l.phase;
     double saw=2*ph-1-blep(ph,increment),width=1.-std::clamp(double(p[PW]+p[PWMDepth]*pwm[b]*.2),.03,.97);
     double shifted=ph-width;if(shifted<0)shifted+=1;double pulse=(ph<width?1.:-1.)+blep(ph,increment)-blep(shifted,increment);
     double source=(p[Saw]>.5?saw*.6:0)+(p[Square]>.5?pulse*.5:0)+p[Noise]*commonNoise*.45;
     double filtered=l.lp.tick(l.hp.tick(soft(source),true),false);
     double sine=std::sin(2*pi*ph); // Analytic, phase-coherent bypass; no audio samples.
     double tone=filtered*p[FilterLevel]+sine*p[Sine]*.55;
     double touch=(1-p[InitialLevel]*.8)+p[InitialLevel]*v.initial*.8+p[AfterLevel]*v.smoothPressure*.9;
     double keyGain=std::exp2((values[LowLevel]*keysLow+values[HighLevel]*keysHigh)*1.5);
     double amplitude=std::max(0.,1+sub*values[SubVCA]*.8);
     banks[b]+=tone*l.ampValue*p[AmpLevel]*touch*keyGain*amplitude*fade;
    }
    if(!v.held&&v.layers[0].amp.stage==Envelope::Off&&v.layers[1].amp.stage==Envelope::Off)v.active=false;
   }
   double mix=values[Mix];double signal=(banks[0]*std::min(1.,2*(1-mix))+banks[1]*std::min(1.,2*mix))*.12;
   float ringEnv=ringEnvelope.tick(1/osRate,values[RingAttack],values[RingDecay],.01,0,0);
   double ringHz=std::min(osRate*.4,double(values[RingSpeed])*std::exp2(ringEnv*values[RingDepth]*5));ringPhase+=ringHz/osRate;ringPhase-=std::floor(ringPhase);
   signal=signal*(1-values[RingMod])+signal*std::sin(2*pi*ringPhase)*values[RingMod];
   firHistory[firIndex]=signal;double decimated=0;for(int j=0;j<63;j++)decimated+=fir[j]*firHistory[(firIndex-j+63)%63];firIndex=(firIndex+1)%63;stereo[os]=decimated;
  }
  double mono=stereo[1]; // 63-tap anti-alias filter before 2:1 decimation.
  effectMix+=(float(values[EffectOn]>.5f)-effectMix)*float(1-std::exp(-dt/.01));
  if(effectMix<1e-9f)effectMix=0;else if(effectMix>1-1e-6f)effectMix=1;
  double fxHz=values[Tremolo]>.5f?5+values[EffectSpeed]*15:.5+values[EffectSpeed]*4.5;effectPhase+=fxHz*dt;effectPhase-=std::floor(effectPhase);
  double mod=std::sin(2*pi*effectPhase),depth=values[EffectDepth];delay[delayIndex]=float(mono);
  double out[2];for(int ch=0;ch<2;ch++){
   double offset=(.006+.0035*depth*(ch?-mod:mod))*sampleRate;offset=std::clamp(offset,1.,16382.);
   double pos=delayIndex-offset;if(pos<0)pos+=delay.size();int at=int(pos);double frac=pos-at;
   double delayed=delay[at]*(1-frac)+delay[(at+1)%delay.size()]*frac;
   double wet=(mono*.55+delayed*.45)*(1-depth*.3*(1+(ch?-mod:mod)))*.9;
   out[ch]=mono*(1-effectMix)+wet*effectMix;
  }delayIndex=(delayIndex+1)%delay.size();
  double gain=values[Volume]*values[Expression]*(values[OutputLow]>.5f?.1:1);
  double coeff=1-std::exp(-2*pi*18000/sampleRate);downL+=coeff*(out[0]-downL);downR+=coeff*(out[1]-downR);
  // DC blocker, no brickwall limiting: leaves the instrument's dynamics intact.
  double l=downL*gain,r=downR*gain,dcCoeff=std::exp(-2*pi*8/sampleRate);dcL=l-prevL+dcCoeff*dcL;dcR=r-prevR+dcCoeff*dcR;prevL=l;prevR=r;
  if(std::abs(downL)<1e-24)downL=0;if(std::abs(downR)<1e-24)downR=0;if(std::abs(dcL)<1e-24)dcL=0;if(std::abs(dcR)<1e-24)dcR=0;
  left[n]=std::isfinite(dcL)?float(dcL):0;right[n]=std::isfinite(dcR)?float(dcR):0;
 }
}
}
