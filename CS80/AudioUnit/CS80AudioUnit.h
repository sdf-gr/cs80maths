#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>
NS_ASSUME_NONNULL_BEGIN
@interface CS80AudioUnit : AUAudioUnit
- (void)panic;
- (BOOL)loadPerformancePreset:(NSInteger)index;
@end
NS_ASSUME_NONNULL_END
