#include "Plugin.hpp"
#include "public.sdk/source/main/pluginfactory.h"
BEGIN_FACTORY_DEF("Direct Grooves","https://directgrooves.com","software@directgrooves.com")
 DEF_CLASS2(INLINE_UID_FROM_FUID(dg::ProcessorUID),Steinberg::PClassInfo::kManyInstances,kVstAudioEffectClass,"cs80maths",Steinberg::Vst::kDistributable,"Instrument|Synth","0.3.0",kVstVersionString,dg::Processor::create)
 DEF_CLASS2(INLINE_UID_FROM_FUID(dg::ControllerUID),Steinberg::PClassInfo::kManyInstances,kVstComponentControllerClass,"cs80maths Controller",0,"","0.3.0",kVstVersionString,dg::Controller::create)
END_FACTORY
