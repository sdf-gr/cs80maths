#import <Cocoa/Cocoa.h>
#import <AudioUnit/AudioUnit.h>
#import <CoreAudioKit/CoreAudioKit.h>
#import "../AudioUnit/CS80AudioUnit.h"
NS_ASSUME_NONNULL_BEGIN
NS_INLINE void CS80SendAllSoundOff(AUAudioUnit*unit) {
 AUScheduleMIDIEventBlock block=unit.scheduleMIDIEventBlock;
 if(block)for(int channel=0;channel<16;channel++){uint8_t bytes[3]={(uint8_t)(0xb0|channel),120,0};block(AUEventSampleTimeImmediate,0,3,bytes);}
}
@interface CS80ViewController : AUViewController <AUAudioUnitFactory>
@property(nonatomic,strong,nullable) AUAudioUnit *unit;
@end
NS_ASSUME_NONNULL_END
