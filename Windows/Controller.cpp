#include "Plugin.hpp"
#include "base/source/fstreamer.h"
#include "base/source/fstring.h"
#include "pluginterfaces/base/ustring.h"
namespace dg {
tresult PLUGIN_API Controller::initialize(FUnknown* context){auto result=EditController::initialize(context);if(result!=kResultOk)return result;
 for(int i=0;i<cs80::ParameterCount;i++){auto&s=specs[i];String title((s.group+" · "+s.name).c_str(),kCP_Utf8);if(!s.choices.empty()){auto*p=new StringListParameter(title.text16(),i);for(auto&c:s.choices){String text(c.c_str(),kCP_Utf8);p->appendString(text.text16());}parameters.addParameter(p);setParamNormalized(i,normalized(s,s.initial));}else parameters.addParameter(title.text16(),nullptr,s.discrete?int32(s.hi-s.lo):0,normalized(s,s.initial),ParameterInfo::kCanAutomate,i);}
 auto*p=new StringListParameter(STR16("Performance preset"),PresetID,nullptr,ParameterInfo::kCanAutomate|ParameterInfo::kIsProgramChange);p->appendString(STR16("Custom / current settings"));for(auto&name:cs80::performanceNames){String s(name,kCP_Utf8);p->appendString(s.text16());}parameters.addParameter(p);
 for(int ch=0;ch<16;ch++)for(int cc=0;cc<130;cc++){String title(("MIDI "+std::to_string(ch+1)+" / "+std::to_string(cc)).c_str());parameters.addParameter(title.text16(),nullptr,0,cc==129?.5:0,ParameterInfo::kIsHidden,midiID(ch,cc));}
 return kResultOk;
}
tresult PLUGIN_API Controller::getMidiControllerAssignment(int32 bus,int16 ch,CtrlNumber number,ParamID&id){if(bus!=0||ch<0||ch>15||number<0||number>129)return kResultFalse;id=midiID(ch,number);return kResultOk;}
tresult PLUGIN_API Controller::setParamNormalized(ParamID id,ParamValue value){auto r=EditController::setParamNormalized(id,value);if(id==PresetID&&r==kResultOk){int index=int(std::round(value*8))-1;if(index>=0&&index<8){auto sound=cs80::performancePreset(index);for(int i=0;i<cs80::ParameterCount;i++){if(i==cs80::MPE||i==cs80::Expression||i==cs80::FootSwitch||i==cs80::PedalConnected||i==cs80::OutputLow||i>=cs80::MemoryBase)continue;EditController::setParamNormalized(i,normalized(specs[i],sound.values[i]));}}}return r;}
void Controller::edit(ParamID id,double v){beginEdit(id);setParamNormalized(id,v);performEdit(id,v);endEdit(id);}
void Controller::choosePreset(int index){if(index<1||index>8)return;edit(PresetID,index/8.);auto sound=cs80::performancePreset(index-1);for(int i=0;i<cs80::ParameterCount;i++){if(i==cs80::MPE||i==cs80::Expression||i==cs80::FootSwitch||i==cs80::PedalConnected||i==cs80::OutputLow||i>=cs80::MemoryBase)continue;edit(i,normalized(specs[i],sound.values[i]));}}
tresult PLUGIN_API Controller::setComponentState(IBStream*stream){IBStreamer s(stream,kLittleEndian);int32 magic=0,version=0,preset=0;std::array<float,cs80::ParameterCount>values{};if(!s.readInt32(magic)||magic!=0x4353384d||!s.readInt32(version)||version!=1||!s.readInt32(preset)||preset<0||preset>8)return kResultFalse;for(auto&v:values)if(!s.readFloat(v)||!std::isfinite(v))return kResultFalse;EditController::setParamNormalized(PresetID,preset/8.);for(int i=0;i<cs80::ParameterCount;i++)EditController::setParamNormalized(i,normalized(specs[i],std::clamp(values[i],specs[i].lo,specs[i].hi)));return kResultOk;}
}
