#include "Plugin.hpp"
#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
namespace dg {
Processor::Processor(){setControllerClass(ControllerUID);}
tresult PLUGIN_API Processor::initialize(FUnknown* host){auto r=AudioEffect::initialize(host);if(r!=kResultOk)return r;addAudioInput(STR16("External modulation"),SpeakerArr::kStereo,kAux,0);addAudioOutput(STR16("Stereo output"),SpeakerArr::kStereo);addEventInput(STR16("MIDI input"),16);return kResultOk;}
tresult PLUGIN_API Processor::setupProcessing(ProcessSetup& setup){if(setup.symbolicSampleSize!=kSample32||setup.sampleRate<8000||setup.sampleRate>384000)return kResultFalse;auto r=AudioEffect::setupProcessing(setup);if(r==kResultOk)engine.prepare(setup.sampleRate);return r;}
tresult PLUGIN_API Processor::setActive(TBool state){engine.reset();return AudioEffect::setActive(state);}
tresult PLUGIN_API Processor::setProcessing(TBool){engine.reset();return kResultOk;}
tresult PLUGIN_API Processor::setBusArrangements(SpeakerArrangement* ins,int32 ni,SpeakerArrangement* outs,int32 no){if(ni!=1||no!=1||ins[0]!=SpeakerArr::kStereo||outs[0]!=SpeakerArr::kStereo)return kResultFalse;return AudioEffect::setBusArrangements(ins,ni,outs,no);}
void Processor::change(ParamID id,double v){
 if(!std::isfinite(v))return;
 if(id<cs80::ParameterCount){engine.eventParameter(int(id),plain(specs[id],v),uint32(processSetup.sampleRate*.005));return;}
 if(id==PresetID){selected=int(std::round(std::clamp(v,0.,1.)*cs80::PerformancePresetCount));if(selected){auto p=cs80::performancePreset(selected-1);for(int i=0;i<cs80::ParameterCount;i++){if(i==cs80::MPE||i==cs80::Expression||i==cs80::FootSwitch||i==cs80::PedalConnected||i==cs80::OutputLow||i>=cs80::MemoryBase)continue;engine.eventParameter(i,p.values[i],uint32(processSetup.sampleRate*.005));}}return;}
 if(id<MidiBase||id>=MidiBase+MidiCount)return;
 int channel=int(id-MidiBase)/130,cc=int(id-MidiBase)%130;
 v=std::clamp(v,0.,1.);
 if(cc==128)engine.midi(uint8_t(0xd0|channel),uint8_t(std::round(v*127)),0);
 else if(cc==129){int bend=int(std::round(v*16383));engine.midi(uint8_t(0xe0|channel),bend&127,(bend>>7)&127);}
 else engine.midi(uint8_t(0xb0|channel),uint8_t(cc),uint8_t(std::round(v*127)));
}
void Processor::note(const Event&e){switch(e.type){case Event::kNoteOnEvent:engine.midi(uint8_t(0x90|e.noteOn.channel),uint8_t(e.noteOn.pitch),uint8_t(std::clamp(int(std::round(e.noteOn.velocity*127)),1,127)));break;case Event::kNoteOffEvent:engine.midi(uint8_t(0x80|e.noteOff.channel),uint8_t(e.noteOff.pitch),0);break;case Event::kPolyPressureEvent:engine.midi(uint8_t(0xa0|e.polyPressure.channel),uint8_t(e.polyPressure.pitch),uint8_t(std::clamp(int(std::round(e.polyPressure.pressure*127)),0,127)));break;default:break;}}
tresult PLUGIN_API Processor::process(ProcessData& data){
 if(data.symbolicSampleSize!=kSample32)return kResultFalse;
 auto* changes=data.inputParameterChanges;int queues=changes?changes->getParameterCount():0;if(queues>MaxQueues)return kResultFalse;
 std::fill_n(cursors.begin(),queues,0);int eventIndex=0,events=data.inputEvents?data.inputEvents->getEventCount():0,pos=0;
 // Merge host queues and MIDI events by sample offset without render-time allocation.
 // Zero-frame calls still flush parameter changes at offset zero.
 while(true){
  int next=data.numSamples;
  for(int q=0;q<queues;q++){auto* queue=changes->getParameterData(q);if(!queue)continue;int32 off=0;double value=0;if(cursors[q]<queue->getPointCount()&&queue->getPoint(cursors[q],off,value)==kResultOk)next=std::min(next,std::clamp(int(off),pos,data.numSamples));}
  Event e{};bool haveEvent=eventIndex<events&&data.inputEvents->getEvent(eventIndex,e)==kResultOk;if(haveEvent)next=std::min(next,std::clamp(int(e.sampleOffset),pos,data.numSamples));
  if(next>pos&&data.numOutputs>0&&data.outputs[0].numChannels==2&&data.outputs[0].channelBuffers32){auto*out=data.outputs[0].channelBuffers32;const float*external=nullptr;if(data.numInputs>0&&data.inputs[0].numChannels>0&&data.inputs[0].channelBuffers32)external=data.inputs[0].channelBuffers32[0];engine.process(out[0]+pos,out[1]+pos,next-pos,external?external+pos:nullptr);data.outputs[0].silenceFlags=0;}
  pos=next;bool consumed=false;
  for(int q=0;q<queues;q++){auto* queue=changes->getParameterData(q);if(!queue)continue;int32 off=0;double value=0;while(cursors[q]<queue->getPointCount()&&queue->getPoint(cursors[q],off,value)==kResultOk&&off<=pos){change(queue->getParameterId(),value);++cursors[q];consumed=true;}}
  while(eventIndex<events&&data.inputEvents->getEvent(eventIndex,e)==kResultOk&&e.sampleOffset<=pos){note(e);++eventIndex;consumed=true;}
  if(pos>=data.numSamples)break;
  if(!consumed&&next==pos)return kResultFalse;
 }
 return kResultOk;
}
tresult PLUGIN_API Processor::getState(IBStream*stream){IBStreamer s(stream,kLittleEndian);if(!s.writeInt32(0x4353384d)||!s.writeInt32(1)||!s.writeInt32(selected))return kResultFalse;for(int i=0;i<cs80::ParameterCount;i++)if(!s.writeFloat(engine.get(i)))return kResultFalse;return kResultOk;}
tresult PLUGIN_API Processor::setState(IBStream*stream){IBStreamer s(stream,kLittleEndian);int32 magic=0,version=0,preset=0;std::array<float,cs80::ParameterCount>values{};if(!s.readInt32(magic)||magic!=0x4353384d||!s.readInt32(version)||version!=1||!s.readInt32(preset)||preset<0||preset>8)return kResultFalse;for(auto&v:values)if(!s.readFloat(v)||!std::isfinite(v))return kResultFalse;for(int i=0;i<cs80::ParameterCount;i++)engine.eventParameter(i,values[i]);selected=preset;engine.reset();return kResultOk;}
}
