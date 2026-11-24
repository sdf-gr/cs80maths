#include "Plugin.hpp"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "public.sdk/source/common/memorystream.h"
#include <windows.h>
#include <iostream>
#include <stdexcept>
using namespace dg;
void check(bool value,const char*message){if(!value)throw std::runtime_error(message);}
int main(){try{
 HostApplication host;auto*p=new Processor;auto*c=new Controller;
 check(p->initialize(&host)==kResultOk&&c->initialize(&host)==kResultOk,"initialize");
 check(c->getParameterCount()==cs80::ParameterCount+1+MidiCount,"parameter count");ParamID id=0;check(c->getMidiControllerAssignment(0,7,128,id)==kResultOk&&id==midiID(7,128),"aftertouch mapping");check(c->getMidiControllerAssignment(0,15,129,id)==kResultOk&&id==midiID(15,129),"pitch mapping");
 for(double rate:{44100.,48000.,96000.}){
  ProcessSetup setup{kRealtime,kSample32,512,rate};check(p->setupProcessing(setup)==kResultOk,"setup");p->setActive(true);p->setProcessing(true);
  EventList events;Event note{};note.type=Event::kNoteOnEvent;note.sampleOffset=128;note.noteOn.channel=0;note.noteOn.pitch=48;note.noteOn.velocity=.8f;events.addEvent(note);
  float left[512]{},right[512]{};float*buffers[]={left,right};AudioBusBuffers output{};output.numChannels=2;output.channelBuffers32=buffers;ProcessData data{};data.processMode=kRealtime;data.symbolicSampleSize=kSample32;data.numSamples=512;data.numOutputs=1;data.outputs=&output;data.inputEvents=&events;
  check(p->process(data)==kResultOk,"render");for(int i=0;i<128;i++)check(left[i]==0&&right[i]==0,"MIDI offset");double energy=0;for(float x:left){check(std::isfinite(x)&&std::abs(x)<1,"audio range");energy+=x*x;}check(energy>0,"audible note");data.inputEvents=nullptr;
  ParameterChanges changes;int32 index=0,point=0;auto*q=changes.addParameterData(PresetID,index);q->addPoint(64,1./8,point);auto*pressure=changes.addParameterData(midiID(0,128),index);pressure->addPoint(80,1,point);pressure->addPoint(180,0,point);data.inputParameterChanges=&changes;check(p->process(data)==kResultOk,"preset/pressure events");data.inputParameterChanges=nullptr;
  MemoryStream state;check(p->getState(&state)==kResultOk,"save state");state.seek(0,IBStream::kIBSeekSet,nullptr);check(c->setComponentState(&state)==kResultOk,"controller state");check(std::abs(c->getParamNormalized(PresetID)-1./8)<1e-6,"preset state");state.seek(0,IBStream::kIBSeekSet,nullptr);check(p->setState(&state)==kResultOk,"restore state");
  data.numSamples=0;data.numOutputs=0;data.outputs=nullptr;ParameterChanges flush;auto*f=flush.addParameterData(cs80::Volume,index);f->addPoint(0,.5,point);data.inputParameterChanges=&flush;check(p->process(data)==kResultOk,"zero frame automation");p->setProcessing(false);p->setActive(false);
 }
 for(int i=1;i<=8;i++){c->choosePreset(i);check(std::abs(c->getParamNormalized(cs80::ToneI)-1)<1e-6,"preset panel route");}
 // Actually create and exercise the Win32 editor on the Windows runner.
 HWND parent=CreateWindowW(L"STATIC",L"cs80maths test",WS_OVERLAPPEDWINDOW,0,0,1300,850,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);check(parent!=nullptr,"test window");
 for(int cycle=0;cycle<2;cycle++){IPlugView*view=c->createView(ViewType::kEditor);check(view&&view->attached(parent,kPlatformTypeHWND)==kResultOk,"native editor attachment");HWND editor=FindWindowExW(parent,nullptr,L"DGcs80Editor",nullptr);check(editor!=nullptr,"editor window");HWND fader=FindWindowExW(editor,nullptr,L"DGcs80Fader",nullptr);check(fader!=nullptr,"fader window");SendMessageW(fader,WM_KEYDOWN,VK_END,0);check(c->getParamNormalized(cs80::panel(0,0))==1,"fader edits parameter");HWND banks=GetDlgItem(editor,2);SendMessageW(banks,CB_SETCURSEL,1,0);SendMessageW(editor,WM_COMMAND,MAKEWPARAM(2,CBN_SELCHANGE),reinterpret_cast<LPARAM>(banks));fader=FindWindowExW(editor,nullptr,L"DGcs80Fader",nullptr);SendMessageW(fader,WM_KEYDOWN,VK_HOME,0);check(c->getParamNormalized(cs80::memory(0,0))==0,"memory editor route");check(view->removed()==kResultOk,"editor removal");view->release();}
 DestroyWindow(parent);c->terminate();p->terminate();c->release();p->release();std::cout<<"PASS Windows VST3 MIDI offsets, presets, pressure/bend mappings, state recall, zero-frame automation, three sample rates, editor/faders/memories and editor reopen\n";return 0;
 }catch(const std::exception&e){std::cerr<<"FAIL "<<e.what()<<"\n";return 1;}}
