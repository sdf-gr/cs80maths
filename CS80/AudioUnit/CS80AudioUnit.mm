#import "CS80AudioUnit.h"
#import <AudioToolbox/AUAudioUnitImplementation.h>
#include "../DSP/Engine.hpp"
#include "../DSP/PerformancePresets.hpp"
#include <vector>
#include <memory>
struct RenderStorage {cs80::Engine engine;std::vector<float>left,right,inLeft,inRight,external;};
@implementation CS80AudioUnit {
 std::unique_ptr<RenderStorage> _storage;
 AUAudioUnitBus *_output,*_input;AUAudioUnitBusArray *_outputs,*_inputs;
 AUAudioUnitPreset *_selectedPreset;
}
- (instancetype)initWithComponentDescription:(AudioComponentDescription)d options:(AudioComponentInstantiationOptions)o error:(NSError**)error {
 if(!(self=[super initWithComponentDescription:d options:o error:error]))return nil;
 _storage=std::make_unique<RenderStorage>();
 AVAudioFormat*format=[[AVAudioFormat alloc]initStandardFormatWithSampleRate:48000 channels:2];
 _output=[[AUAudioUnitBus alloc]initWithFormat:format error:error];if(!_output)return nil;_output.maximumChannelCount=2;
 _input=[[AUAudioUnitBus alloc]initWithFormat:format error:error];if(!_input)return nil;_input.maximumChannelCount=2;
 _outputs=[[AUAudioUnitBusArray alloc]initWithAudioUnit:self busType:AUAudioUnitBusTypeOutput busses:@[_output]];
 _inputs=[[AUAudioUnitBusArray alloc]initWithAudioUnit:self busType:AUAudioUnitBusTypeInput busses:@[_input]];
 self.maximumFramesToRender=4096;
 auto specs=cs80::makeSpecs();NSMutableArray*groups=[NSMutableArray array];int gi=0;
 // Groups need not be adjacent in the stable address map.
 NSMutableDictionary<NSString*,NSMutableArray*>*byGroup=[NSMutableDictionary dictionary];NSMutableArray<NSString*>*order=[NSMutableArray array];
 for(int i=0;i<cs80::ParameterCount;i++){auto&s=specs[i];NSString*g=@(s.group.c_str());if(!byGroup[g]){byGroup[g]=[NSMutableArray array];[order addObject:g];}
  NSMutableArray*strings=nil;if(!s.choices.empty()){strings=[NSMutableArray array];for(auto&c:s.choices)[strings addObject:@(c.c_str())];}
  AudioUnitParameterOptions flags=kAudioUnitParameterFlag_IsReadable|kAudioUnitParameterFlag_IsWritable;if(!s.discrete)flags|=kAudioUnitParameterFlag_CanRamp;
  AUParameter*p=[AUParameterTree createParameterWithIdentifier:@(s.id.c_str()) name:@(s.name.c_str()) address:i min:s.lo max:s.hi unit:s.discrete?(strings?kAudioUnitParameterUnit_Indexed:kAudioUnitParameterUnit_Boolean):kAudioUnitParameterUnit_Generic unitName:nil flags:flags valueStrings:strings dependentParameters:nil];
  p.value=s.initial;[byGroup[g] addObject:p];
 }
 for(NSString*g in order)[groups addObject:[AUParameterTree createGroupWithIdentifier:[NSString stringWithFormat:@"group%d",gi++] name:g children:byGroup[g]]];
 self.parameterTree=[AUParameterTree createTreeWithChildren:groups];
 auto*engine=&_storage->engine;
 self.parameterTree.implementorValueObserver=^(AUParameter*p,AUValue value){engine->set(int(p.address),value);};
 self.parameterTree.implementorValueProvider=^AUValue(AUParameter*p){return engine->get(int(p.address));};
 self.parameterTree.implementorStringFromValueCallback=^NSString*(AUParameter*p,const AUValue*v){float f=v?*v:p.value;if(p.valueStrings.count)return p.valueStrings[std::clamp((int)std::round(f),0,(int)p.valueStrings.count-1)];return [NSString stringWithFormat:@"%.3g",f];};
 return self;
}
- (AUAudioUnitBusArray*)outputBusses{return _outputs;}
- (AUAudioUnitBusArray*)inputBusses{return _inputs;}
- (uint32_t)componentVersion{return 0x00010400;}
- (BOOL)isMusicDeviceOrEffect{return YES;}
- (BOOL)canProcessInPlace{return NO;}
- (MIDIProtocolID)audioUnitMIDIProtocol{return kMIDIProtocol_1_0;}
- (NSTimeInterval)tailTime{return 10.1;}
- (NSTimeInterval)latency{return 15.5/_output.format.sampleRate;}
- (NSArray<NSNumber*>*)channelCapabilities{return @[@1,@1,@1,@2,@2,@1,@2,@2];}
- (BOOL)allocateRenderResourcesAndReturnError:(NSError**)error {
 AVAudioFormat*f=_output.format;
 if(_input.format.isInterleaved||_input.format.commonFormat!=AVAudioPCMFormatFloat32||_input.format.channelCount<1||_input.format.channelCount>2||_input.format.sampleRate!=f.sampleRate||f.isInterleaved||f.commonFormat!=AVAudioPCMFormatFloat32||f.channelCount<1||f.channelCount>2){if(error)*error=[NSError errorWithDomain:NSOSStatusErrorDomain code:kAudioUnitErr_FormatNotSupported userInfo:nil];return NO;}
 auto&s=*_storage;auto frames=self.maximumFramesToRender;s.left.resize(frames);s.right.resize(frames);s.inLeft.resize(frames);s.inRight.resize(frames);s.external.resize(frames);s.engine.prepare(f.sampleRate);
 return [super allocateRenderResourcesAndReturnError:error];
}
- (void)dealloc{self.parameterTree.implementorValueObserver=^(AUParameter*p,AUValue value){};self.parameterTree.implementorValueProvider=^AUValue(AUParameter*p){return 0;};}
- (void)deallocateRenderResources{[super deallocateRenderResources];}
- (void)reset{_storage->engine.reset();[super reset];}
- (void)panic {auto b=self.scheduleMIDIEventBlock;if(b)for(int c=0;c<16;c++){uint8_t m[3]={(uint8_t)(0xb0|c),120,0};b(AUEventSampleTimeImmediate,0,3,m);}}
static void applyEvent(cs80::Engine&e,const AURenderEvent*event){
 switch(event->head.eventType){case AURenderEventParameter:case AURenderEventParameterRamp:e.eventParameter((int)event->parameter.parameterAddress,event->parameter.value,event->head.eventType==AURenderEventParameterRamp?event->parameter.rampDurationSampleFrames:0);break;
 case AURenderEventMIDI:if(event->MIDI.length>=1){uint8_t st=event->MIDI.data[0];int required=((st&0xf0)==0xc0||(st&0xf0)==0xd0)?2:3;if(event->MIDI.length>=required)e.midi(st,event->MIDI.data[1],required==3?event->MIDI.data[2]:0);}break;
 case AURenderEventMIDIEventList:{const MIDIEventList&list=event->MIDIEventsList.eventList;const MIDIEventPacket*p=&list.packet[0];for(uint32_t j=0;j<list.numPackets;j++){
 for(uint32_t i=0;i<p->wordCount;){uint32_t w=p->words[i],type=w>>28;if(type==2)e.midi((w>>16)&255,(w>>8)&127,w&127);uint32_t len=(type==3||type==4)?2:(type==5||type==0xd||type==0xf)?4:1;i+=len;}p=MIDIEventPacketNext(p);}break;}
 default:break;}
}
- (AUInternalRenderBlock)internalRenderBlock {
 RenderStorage*s=_storage.get();uint32_t maximum=self.maximumFramesToRender;uint32_t channels=_output.format.channelCount;uint32_t inputChannels=_input.format.channelCount;
 return ^AUAudioUnitStatus(AudioUnitRenderActionFlags*flags,const AudioTimeStamp*time,AUAudioFrameCount frames,NSInteger bus,AudioBufferList*out,const AURenderEvent*events,AURenderPullInputBlock pull){
  if(bus!=0)return kAudioUnitErr_InvalidElement;if(frames>maximum||frames>s->left.size())return kAudioUnitErr_TooManyFramesToProcess;
  if(out->mNumberBuffers<channels)return kAudioUnitErr_FormatNotSupported;
  std::fill_n(s->external.data(),frames,0.f);
  if(pull){struct {UInt32 count;AudioBuffer b[2];}input={inputChannels,{{1,frames*4,s->inLeft.data()},{1,frames*4,s->inRight.data()}}};AudioUnitRenderActionFlags inputFlags=0;
   if(pull(&inputFlags,time,frames,0,(AudioBufferList*)&input)==noErr){for(uint32_t i=0;i<frames;i++)s->external[i]=((float*)input.b[0].mData)[i];}}
  uint32_t offset=0;const AURenderEvent*event=events;
  while(offset<frames){
   while(event&&(event->head.eventSampleTime==AUEventSampleTimeImmediate||event->head.eventSampleTime<=time->mSampleTime+offset)){applyEvent(s->engine,event);event=event->head.next;}
   uint32_t end=frames;if(event){double delta=event->head.eventSampleTime-time->mSampleTime;end=(uint32_t)std::clamp(delta,double(offset),double(frames));}if(end==offset)end=offset+1;
   s->engine.process(s->left.data()+offset,s->right.data()+offset,end-offset,s->external.data()+offset);offset=end;
  }
  for(uint32_t c=0;c<channels;c++){if(!out->mBuffers[c].mData)out->mBuffers[c].mData=(c?s->right.data():s->left.data());if(channels==1){float*d=(float*)out->mBuffers[c].mData;for(uint32_t i=0;i<frames;i++)d[i]=(s->left[i]+s->right[i])*.5f;}else memcpy(out->mBuffers[c].mData,c?s->right.data():s->left.data(),frames*sizeof(float));out->mBuffers[c].mDataByteSize=frames*sizeof(float);}
  *flags&=~kAudioUnitRenderAction_OutputIsSilence;return noErr;
 };
}
- (NSDictionary*)fullState {
 NSMutableDictionary*state=[[super fullState]mutableCopy]?:[NSMutableDictionary dictionary];NSMutableArray*values=[NSMutableArray arrayWithCapacity:cs80::ParameterCount];for(int i=0;i<cs80::ParameterCount;i++)[values addObject:@(_storage->engine.get(i))];state[@"CS80Version"]=@1;state[@"CS80Parameters"]=values;if(_selectedPreset)state[@"CS80SelectedPreset"]=@(_selectedPreset.number);return state;
}
- (void)setFullState:(NSDictionary*)state {
 [super setFullState:state];id data=state[@"CS80Parameters"];if([data isKindOfClass:NSArray.class]){NSArray*a=data;for(NSUInteger i=0;i<MIN(a.count,cs80::ParameterCount);i++)if([a[i]isKindOfClass:NSNumber.class]){float value=[a[i]floatValue];if(std::isfinite(value))[[self.parameterTree parameterWithAddress:i]setValue:value];}}
 id selected=state[@"CS80SelectedPreset"];if([selected isKindOfClass:NSNumber.class]){NSInteger n=[selected integerValue];NSArray*presets=self.factoryPresets;_selectedPreset=n>=0&&n<(NSInteger)presets.count?presets[n]:nil;}else _selectedPreset=nil;
}
- (NSArray<AUAudioUnitPreset*>*)factoryPresets {NSMutableArray*a=[NSMutableArray array];NSArray*names=@[@"Layered strings",@"Dark strings",@"Brass",@"Flute / brass",@"Piano / bass",@"Clavichords",@"Harpsichords",@"Organs",@"Guitars",@"Funky I",@"Funky II"];
 for(int i=0;i<11;i++){AUAudioUnitPreset*p=[AUAudioUnitPreset new];p.number=i;p.name=names[i];[a addObject:p];}for(int i=0;i<cs80::PerformancePresetCount;i++){AUAudioUnitPreset*p=[AUAudioUnitPreset new];p.number=11+i;p.name=@(cs80::performanceNames[i]);[a addObject:p];}return a;}
- (AUAudioUnitPreset*)currentPreset{return _selectedPreset;}
- (void)setCurrentPreset:(AUAudioUnitPreset*)preset{if(!preset){_selectedPreset=nil;return;}if(preset.number<0){_selectedPreset=preset;return;}if(preset.number>=11){[self loadPerformancePreset:preset.number-11];return;}_selectedPreset=preset;[self.parameterTree parameterWithAddress:cs80::ToneI].value=preset.number;[self.parameterTree parameterWithAddress:cs80::ToneII].value=preset.number;}
- (BOOL)loadPerformancePreset:(NSInteger)index {
 if(index<0||index>=cs80::PerformancePresetCount)return NO;
 auto sound=cs80::performancePreset((int)index);
 // Preserve the user's memories and physical controller/routing state.
 for(int address=cs80::PanelBase;address<cs80::MemoryBase;address++)[self.parameterTree parameterWithAddress:address].value=sound.values[address];
 for(int address=0;address<cs80::GlobalCount;address++){
  if(address==cs80::MPE||address==cs80::FootSwitch||address==cs80::PedalConnected||address==cs80::Expression||address==cs80::OutputLow)continue;
  [self.parameterTree parameterWithAddress:address].value=sound.values[address];
 }
 AUAudioUnitPreset*p=[AUAudioUnitPreset new];p.number=11+index;p.name=@(cs80::performanceNames[index]);_selectedPreset=p;return YES;
}
@end
