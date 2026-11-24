#pragma once
#include "Parameters.hpp"
#include <atomic>
#include <cstdint>
namespace cs80 {
struct Envelope {
 enum Stage{Off,Attack,Decay,Hold,Release};Stage stage=Off;float value=0,start=0,target=0,curve=1;double elapsed=0,duration=0;
 void begin(Stage s,float destination){stage=s;start=value;target=destination;elapsed=0;duration=0;curve=1;}
 void trigger(float initial,float peak){value=initial;begin(Attack,peak);}
 float tick(double dt,float a,float d,float r,float sustain,float rest,float peak=1){
  if(stage==Off)return value=rest;if(stage==Hold)return value=sustain;
  float seconds=stage==Attack?a:(stage==Decay?d:r);float destination=stage==Attack?peak:(stage==Decay?sustain:rest);
  if(destination!=target){start=value;target=destination;elapsed=0;duration=0;curve=1;}
  // A changed release time must start from the current level, not reinterpret
  // elapsed time using a new duration (which can jump straight to silence).
  if(duration!=seconds){start=value;elapsed=0;duration=seconds;curve=1;}
  elapsed+=dt;curve=std::exp(-5.f*float(elapsed/std::max(.0001f,seconds)));
  value=target+(start-target)*(curve-.006737947f)/.993262053f;
  if(elapsed>=seconds){value=target;if(stage==Attack)begin(Decay,sustain);else if(stage==Decay)stage=Hold;else stage=Off;}
  return value;
 }
};
struct Filter {
 double s1=0,s2=0,g=.1,k=1.4,a1=1,a2=0,a3=0;
 double targetG=.1,targetK=1.4,stepG=0,stepK=0;uint32_t remaining=0;
 void tune(double hz,double resonance,double sampleRate){
  targetG=std::tan(3.141592653589793*std::clamp(hz,8.,sampleRate*.42)/sampleRate);
  targetK=1.41421356*(1.-.93*std::clamp(resonance,0.,1.));
  // The control target updates every 16 output samples. Interpolate the TPT
  // integrator parameters at 2x audio rate rather than jumping coefficients.
  remaining=32;stepG=(targetG-g)/remaining;stepK=(targetK-k)/remaining;
 }
 double tick(double x,bool high){
  if(remaining){g+=stepG;k+=stepK;if(!--remaining){g=targetG;k=targetK;}}
  a1=1/(1+g*(g+k));a2=g*a1;a3=g*a2;
  double v3=x-s2,v1=a1*s1+a2*v3,v2=s2+a2*s1+a3*v3;s1=2*v1-s1;s2=2*v2-s2;return high?x-k*v1-v2:v2;
 }
};
struct Layer {double phase=0;Envelope amp,filter;Filter hp,lp;float pulse=0,env=0,ampValue=0;};
struct Voice {bool active=false,held=false;int note=60,channel=0,circuit=0;
 uint32_t fadeInLeft=0,fadeOutLeft=0,fadeTotal=0;float fadeStart=1,lastFadeGain=1;float velocity=0,pressure=0,smoothPressure=0,initial=0;double key=60,target=60,ribbon=1;std::array<Layer,2>layers;};
struct Ramp {float step=0;uint32_t left=0;};
class Engine {
public:
 Engine();void prepare(double sampleRate);void reset();void set(int address,float value);float get(int address)const;
 void eventParameter(int address,float value,uint32_t frames=0);
 void midi(uint8_t status,uint8_t data1,uint8_t data2);void process(float*left,float*right,uint32_t frames,const float*external=nullptr);
 int heldCount()const;int activeCount()const;const Voice&voice(int i)const{return voices[i];}
 double rate()const{return sampleRate;}
private:
 std::vector<Spec>specs;std::array<std::atomic<float>,ParameterCount>published;
 std::array<float,ParameterCount>values,seen;std::array<Ramp,ParameterCount>ramps;
 std::array<Voice,8>voices;std::array<Voice,16>retiredVoices;std::array<float,16>pressure{},bend{},smoothBend{};std::array<std::array<int,128>,16>ignored{};
 std::array<PatchValues,2>patch;std::array<double,2>pwmPhase{};std::array<float,2>pwm{};
 double pressureSlew=.005,bendSlew=.01;
 double sampleRate=48000,subPhase=0,ringPhase=0,effectPhase=0;int nextVoice=0,controlCounter=0;
 Envelope ringEnvelope;uint32_t randomState=0x80c580;std::array<float,16384>delay{};int delayIndex=0;
 std::array<double,63>fir{},firHistory{};int firIndex=0;
 double downL=0,downR=0,dcL=0,dcR=0,prevL=0,prevR=0;float effectMix=0;
 float random();void noteOn(int note,float velocity,int channel);void noteOff(int note,int channel);void release(Voice&v);void retire(const Voice&v);bool pedal()const;bool sustainOn()const;bool glideOn()const;void updatePatch();void controls();void tickParameters();
};
}
