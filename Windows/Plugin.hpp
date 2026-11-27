#pragma once
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "CS80/DSP/Engine.hpp"
#include "CS80/DSP/PerformancePresets.hpp"
#include <array>
namespace dg {
using namespace Steinberg;
using namespace Steinberg::Vst;
inline const FUID ProcessorUID(0x88B04B21,0xECFA4698,0x99070580,0xDB816301);
inline const FUID ControllerUID(0x9623D02A,0x490447E2,0x8CF82780,0x87B3A8E6);
constexpr ParamID PresetID=cs80::ParameterCount;
constexpr ParamID AdditionalPresetID=PresetID+1;
constexpr int AdditionalPresetCount=cs80::PerformancePresetCount-cs80::LegacyPerformancePresetCount;
constexpr ParamID MidiBase=1000;
constexpr int MidiCount=16*130;
constexpr int MaxQueues=MidiBase+MidiCount;
inline ParamID midiID(int channel,int number){return MidiBase+channel*130+number;}
inline double normalized(const cs80::Spec&s,float v){return (v-s.lo)/(s.hi-s.lo);}
inline float plain(const cs80::Spec&s,double v){float x=float(s.lo+std::clamp(v,0.,1.)*(s.hi-s.lo));return s.discrete?std::round(x):x;}
class Processor:public AudioEffect {
public:
 Processor();static FUnknown* create(void*){return static_cast<IAudioProcessor*>(new Processor);}
 tresult PLUGIN_API initialize(FUnknown*) override;
 tresult PLUGIN_API setupProcessing(ProcessSetup&) override;
 tresult PLUGIN_API setActive(TBool) override;
 tresult PLUGIN_API setProcessing(TBool) override;
 tresult PLUGIN_API setBusArrangements(SpeakerArrangement*,int32,SpeakerArrangement*,int32) override;
 tresult PLUGIN_API canProcessSampleSize(int32 size) override{return size==kSample32?kResultTrue:kResultFalse;}
 tresult PLUGIN_API process(ProcessData&) override;
 tresult PLUGIN_API getState(IBStream*) override;
 tresult PLUGIN_API setState(IBStream*) override;
 uint32 PLUGIN_API getLatencySamples() override{return 16;}
 uint32 PLUGIN_API getTailSamples() override{return uint32(processSetup.sampleRate*12);}
private:
 cs80::Engine engine;std::vector<cs80::Spec> specs=cs80::makeSpecs();int selected=0;
 std::array<int32,MaxQueues> cursors{};
 void change(ParamID,double);void note(const Event&);
};
class Controller:public EditController,public IMidiMapping {
public:
 static FUnknown* create(void*){return static_cast<IEditController*>(new Controller);}
 tresult PLUGIN_API initialize(FUnknown*) override;
 tresult PLUGIN_API setComponentState(IBStream*) override;
 tresult PLUGIN_API setParamNormalized(ParamID,ParamValue) override;
 tresult PLUGIN_API getMidiControllerAssignment(int32 bus,int16 channel,CtrlNumber number,ParamID& id) override;
 IPlugView* PLUGIN_API createView(FIDString) override;
 int selectedPreset(){int n=int(std::round(getParamNormalized(AdditionalPresetID)*AdditionalPresetCount));return n?n+8:int(std::round(getParamNormalized(PresetID)*8));}
 void choosePreset(int index);void edit(ParamID,double);
 OBJ_METHODS(Controller,EditController)
 DEFINE_INTERFACES
  DEF_INTERFACE(IMidiMapping)
 END_DEFINE_INTERFACES(EditController)
 REFCOUNT_METHODS(EditController)
private:
 std::vector<cs80::Spec> specs=cs80::makeSpecs();
};
}
