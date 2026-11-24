#import <Cocoa/Cocoa.h>
#include <algorithm>
#import <AVFoundation/AVFoundation.h>
#import <CoreMIDI/CoreMIDI.h>
#import "../UI/CS80ViewController.h"
#import <AudioToolbox/AUAudioUnitImplementation.h>
@interface CS80AppDelegate:NSObject<NSApplicationDelegate,NSToolbarDelegate>
@end
@implementation CS80AppDelegate {
 NSWindow*_window;CS80ViewController*_controller;AVAudioEngine*_engine;AVAudioUnit*_instrument;
 NSButton*_audio;NSPopUpButton*_sources;NSTextField*_status;MIDIClientRef _client;MIDIPortRef _port;MIDIEndpointRef _source;
 uint8_t _running,_data[2];int _received;
}
- (void)applicationDidFinishLaunching:(NSNotification*)n {
 NSMenu*bar=[NSMenu new];NSMenuItem*app=[NSMenuItem new];[bar addItem:app];NSMenu*menu=[NSMenu new];[menu addItemWithTitle:@"Quit cs80maths" action:@selector(terminate:) keyEquivalent:@"q"];app.submenu=menu;NSApp.mainMenu=bar;
 _controller=[CS80ViewController new];
 _window=[[NSWindow alloc]initWithContentRect:NSMakeRect(80,60,1340,812) styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:NO];_window.title=@"cs80maths — Direct Grooves";_window.minSize=NSMakeSize(1060,680);NSRect screen=NSScreen.mainScreen.visibleFrame;[_window setContentSize:NSMakeSize(std::min(1340.,screen.size.width-60),std::min(812.,screen.size.height-100))];_window.contentViewController=_controller;
 NSToolbar*toolbar=[[NSToolbar alloc]initWithIdentifier:@"transport"];toolbar.delegate=self;toolbar.displayMode=NSToolbarDisplayModeIconOnly;_window.toolbar=toolbar;[_window center];[_window makeKeyAndOrderFront:nil];[NSApp activateIgnoringOtherApps:YES];
 AudioComponentDescription d={kAudioUnitType_MusicDevice,'Cs80','DjAz',0,0};[AUAudioUnit registerSubclass:CS80AudioUnit.class asComponentDescription:d name:@"Direct Grooves: cs80maths" version:0x00010300];
 [AVAudioUnit instantiateWithComponentDescription:d options:0 completionHandler:^(AVAudioUnit*u,NSError*error){dispatch_async(dispatch_get_main_queue(),^{if(error){[self showError:error];return;}self->_instrument=u;self->_controller.unit=u.AUAudioUnit;self->_engine=[AVAudioEngine new];[self->_engine attachNode:u];[self->_engine connect:u to:self->_engine.mainMixerNode format:nil];self->_status.stringValue=@"Ready — start audio to play";});}];
 __weak CS80AppDelegate*weak=self;
 MIDIClientCreateWithBlock(CFSTR("cs80maths"),&_client,^(const MIDINotification*note){dispatch_async(dispatch_get_main_queue(),^{[weak refreshSources:nil];});});
 MIDIInputPortCreateWithBlock(_client,CFSTR("CS-80 input"),&_port,^(const MIDIPacketList*list,void*connection){CS80AppDelegate*s=weak;if(!s)return;const MIDIPacket*p=&list->packet[0];for(UInt32 i=0;i<list->numPackets;i++){for(UInt16 j=0;j<p->length;j++)[s midiByte:p->data[j]];p=MIDIPacketNext(p);}});
 [self refreshSources:nil];
}
- (void)midiByte:(uint8_t)b {
 if(b>=0xf8)return;if(b&0x80){if(b>=0xf0){_running=0;_received=0;return;}_running=b;_received=0;return;}if(!_running)return;
 _data[_received++]=b;int needed=((_running&0xf0)==0xc0||(_running&0xf0)==0xd0)?1:2;if(_received>=needed){auto block=_instrument.AUAudioUnit.scheduleMIDIEventBlock;if(block){uint8_t bytes[3]={_running,_data[0],_data[1]};block(AUEventSampleTimeImmediate,0,needed+1,bytes);}_received=0;}
}
- (NSArray*)toolbarAllowedItemIdentifiers:(NSToolbar*)t{return @[@"audio",@"sources",@"refresh",@"save",@"load",@"status",NSToolbarFlexibleSpaceItemIdentifier];}
- (NSArray*)toolbarDefaultItemIdentifiers:(NSToolbar*)t{return @[@"audio",@"sources",@"refresh",@"save",@"load",NSToolbarFlexibleSpaceItemIdentifier,@"status"];}
- (NSToolbarItem*)toolbar:(NSToolbar*)t itemForItemIdentifier:(NSString*)identifier willBeInsertedIntoToolbar:(BOOL)flag{
 NSToolbarItem*item=[[NSToolbarItem alloc]initWithItemIdentifier:identifier];
 if([identifier isEqual:@"audio"]){_audio=[NSButton buttonWithTitle:@"Start Audio" target:self action:@selector(toggleAudio:)];item.view=_audio;}
 else if([identifier isEqual:@"sources"]){_sources=[[NSPopUpButton alloc]initWithFrame:NSMakeRect(0,0,250,28) pullsDown:NO];_sources.target=self;_sources.action=@selector(selectSource:);item.view=_sources;}
 else if([identifier isEqual:@"status"]){_status=[NSTextField labelWithString:@"Loading instrument…"];_status.frame=NSMakeRect(0,0,290,22);item.view=_status;}
 else{SEL action=[identifier isEqual:@"refresh"]?@selector(refreshSources:):[identifier isEqual:@"save"]?@selector(saveState:):@selector(loadState:);NSString*title=[identifier isEqual:@"refresh"]?@"Refresh MIDI":[identifier isEqual:@"save"]?@"Save sound…":@"Load sound…";item.view=[NSButton buttonWithTitle:title target:self action:action];}return item;
}
- (void)toggleAudio:(id)sender{if(!_engine)return;if(_engine.running){[_engine stop];_audio.title=@"Start Audio";_status.stringValue=@"Audio stopped";}else{NSError*error=nil;[_engine prepare];if([_engine startAndReturnError:&error]){_audio.title=@"Stop Audio";_status.stringValue=@"Audio running";}else [self showError:error];}}
- (void)refreshSources:(id)sender {if(!_sources)return;MIDIEndpointRef old=_source;[_sources removeAllItems];[_sources addItemWithTitle:@"MIDI input: none"];_sources.lastItem.tag=0;for(ItemCount i=0;i<MIDIGetNumberOfSources();i++){MIDIEndpointRef endpoint=MIDIGetSource(i);CFStringRef name=nil;MIDIObjectGetStringProperty(endpoint,kMIDIPropertyDisplayName,&name);[_sources addItemWithTitle:(__bridge NSString*)name?:@"MIDI source"];_sources.lastItem.tag=endpoint;if(name)CFRelease(name);}BOOL exists=NO;for(NSMenuItem*i in _sources.itemArray)if(i.tag==old){[_sources selectItem:i];exists=YES;break;}if(!exists){if(_source)MIDIPortDisconnectSource(_port,_source);_source=0;} }
- (void)selectSource:(id)sender {if(_source)MIDIPortDisconnectSource(_port,_source);CS80SendAllSoundOff(_controller.unit);_running=0;_received=0;_source=(MIDIEndpointRef)_sources.selectedItem.tag;if(_source)MIDIPortConnectSource(_port,_source,nullptr);}
- (void)saveState:(id)sender{if(!_controller.unit)return;NSSavePanel*p=[NSSavePanel savePanel];p.nameFieldStringValue=@"CS80 Sound.aupreset";[p beginSheetModalForWindow:_window completionHandler:^(NSModalResponse response){if(response==NSModalResponseOK){NSError*error=nil;NSData*d=[NSPropertyListSerialization dataWithPropertyList:self->_controller.unit.fullState format:NSPropertyListXMLFormat_v1_0 options:0 error:&error];if(d)[d writeToURL:p.URL options:NSDataWritingAtomic error:&error];if(error)[self showError:error];}}];}
- (void)loadState:(id)sender{NSOpenPanel*p=[NSOpenPanel openPanel];p.allowsMultipleSelection=NO;[p beginSheetModalForWindow:_window completionHandler:^(NSModalResponse response){if(response==NSModalResponseOK){NSError*error=nil;NSData*d=[NSData dataWithContentsOfURL:p.URL options:0 error:&error];id state=d?[NSPropertyListSerialization propertyListWithData:d options:NSPropertyListImmutable format:nil error:&error]:nil;if([state isKindOfClass:NSDictionary.class])self->_controller.unit.fullState=state;else if(!error)error=[NSError errorWithDomain:@"CS80" code:1 userInfo:@{NSLocalizedDescriptionKey:@"This file does not contain an Audio Unit preset."}];if(error)[self showError:error];}}];}
- (void)showError:(NSError*)error{if(!error)return;_status.stringValue=error.localizedDescription;NSAlert*a=[NSAlert alertWithError:error];[a beginSheetModalForWindow:_window completionHandler:nil];}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)app{return YES;}
- (void)applicationWillTerminate:(NSNotification*)n{[_engine stop];if(_port)MIDIPortDispose(_port);if(_client)MIDIClientDispose(_client);}
@end
int main(int argc,const char*argv[]){@autoreleasepool{NSApplication*app=[NSApplication sharedApplication];[app setActivationPolicy:NSApplicationActivationPolicyRegular];CS80AppDelegate*delegate=[CS80AppDelegate new];app.delegate=delegate;[app run];}return 0;}
