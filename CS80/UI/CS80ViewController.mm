#import "CS80ViewController.h"
#include "../DSP/Parameters.hpp"
#include "../DSP/PerformancePresets.hpp"

static NSColor* ink(){return [NSColor colorWithWhite:.84 alpha:1];}
static NSColor* panel(){return [NSColor colorWithWhite:.115 alpha:1];}
static NSColor* cap(int kind){
 switch(kind){case 1:return [NSColor colorWithRed:.36 green:.57 blue:.36 alpha:1];case 2:return [NSColor colorWithRed:.68 green:.24 blue:.2 alpha:1];case 3:return [NSColor colorWithRed:.83 green:.69 blue:.29 alpha:1];case 4:return [NSColor colorWithWhite:.85 alpha:1];case 5:return [NSColor colorWithWhite:.5 alpha:1];default:return [NSColor colorWithWhite:.23 alpha:1];}
}
static NSTextField* label(NSView*parent,NSString*text,NSRect frame,CGFloat size,NSColor*color=ink(),NSTextAlignment align=NSTextAlignmentCenter){
 NSTextField*t=[NSTextField labelWithString:text];t.frame=frame;t.font=[NSFont systemFontOfSize:size weight:NSFontWeightMedium];t.textColor=color;t.alignment=align;t.lineBreakMode=NSLineBreakByClipping;t.maximumNumberOfLines=2;[parent addSubview:t];return t;
}
static void drawText(NSString*text,NSRect rect,CGFloat size,NSColor*color=ink(),NSTextAlignment alignment=NSTextAlignmentCenter){
 NSMutableParagraphStyle*p=[NSMutableParagraphStyle new];p.alignment=alignment;
 [text drawInRect:rect withAttributes:@{NSFontAttributeName:[NSFont systemFontOfSize:size weight:NSFontWeightMedium],NSForegroundColorAttributeName:color,NSParagraphStyleAttributeName:p}];
}
@interface CS80Surface:NSView
@property NSString*heading;
@end
@implementation CS80Surface
- (void)drawRect:(NSRect)dirty{NSRect r=NSInsetRect(self.bounds,.5,.5);[panel()setFill];[[NSBezierPath bezierPathWithRoundedRect:r xRadius:3 yRadius:3]fill];[[NSColor colorWithWhite:.34 alpha:1]setStroke];[[NSBezierPath bezierPathWithRoundedRect:r xRadius:3 yRadius:3]stroke];if(self.heading.length)drawText(self.heading,NSMakeRect(8,r.size.height-22,r.size.width-16,16),9.5);}
@end
@interface CS80Board:NSView @end
@implementation CS80Board
- (void)drawRect:(NSRect)dirty{
 [[NSColor colorWithWhite:.075 alpha:1]setFill];NSRectFill(self.bounds);
 for(CGFloat x:{CGFloat(0),CGFloat(1318)}){NSGradient*g=[[NSGradient alloc]initWithStartingColor:[NSColor colorWithRed:.2 green:.105 blue:.055 alpha:1] endingColor:[NSColor colorWithRed:.39 green:.22 blue:.115 alpha:1]];[g drawInRect:NSMakeRect(x,0,22,812) angle:0];for(int i=0;i<16;i++){[[NSColor colorWithWhite:0 alpha:.13]setStroke];NSBezierPath*p=[NSBezierPath bezierPath];[p moveToPoint:NSMakePoint(x+2+(i*7)%18,0)];[p lineToPoint:NSMakePoint(x+2+(i*7)%18+std::sin(i)*2,812)];[p stroke];}}
 for(int i=0;i<2;i++)for(int j=0;j<2;j++){NSRect r=NSMakeRect(i?1303:29,j?798:8,7,7);[[NSColor colorWithWhite:.35 alpha:1]setFill];[[NSBezierPath bezierPathWithOvalInRect:r]fill];}
}
@end
@interface CS80ScaledRoot:NSView
@property NSView*board;
@end
@implementation CS80ScaledRoot
- (void)layout{[super layout];if(!self.board)return;CGFloat scale=std::min(self.bounds.size.width/1340.,self.bounds.size.height/812.);self.board.frame=NSMakeRect((self.bounds.size.width-1340*scale)*.5,(self.bounds.size.height-812*scale)*.5,1340*scale,812*scale);self.board.bounds=NSMakeRect(0,0,1340,812);}
@end
@interface CS80FaderCell:NSSliderCell
@property NSColor*capColor;
@end
@implementation CS80FaderCell
- (NSRect)knobRectFlipped:(BOOL)flipped{NSRect r=[super knobRectFlipped:flipped];return NSMakeRect(NSMidX(r)-11.5,NSMidY(r)-6,23,12);}
- (void)drawBarInside:(NSRect)r flipped:(BOOL)flipped{
 CGFloat x=NSMidX(r);[[NSColor colorWithWhite:.035 alpha:1]setFill];NSRectFill(NSMakeRect(x-2,r.origin.y,4,r.size.height));
 [[NSColor colorWithWhite:.42 alpha:1]setStroke];NSBezierPath*p=[NSBezierPath bezierPath];p.lineWidth=.65;for(int i=0;i<=10;i++){CGFloat y=r.origin.y+i*r.size.height/10;CGFloat len=i%5==0?8:5;[p moveToPoint:NSMakePoint(x-5-len,y)];[p lineToPoint:NSMakePoint(x-5,y)];[p moveToPoint:NSMakePoint(x+5,y)];[p lineToPoint:NSMakePoint(x+5+len,y)];}[p stroke];
}
- (void)drawKnob:(NSRect)r{
 NSBezierPath*shape=[NSBezierPath bezierPathWithRoundedRect:r xRadius:1.4 yRadius:1.4];[[NSColor blackColor]setFill];[[NSBezierPath bezierPathWithRoundedRect:NSOffsetRect(r,0,-1.3) xRadius:1.4 yRadius:1.4]fill];
 NSGradient*g=[[NSGradient alloc]initWithStartingColor:[self.capColor blendedColorWithFraction:.22 ofColor:NSColor.whiteColor] endingColor:self.capColor];[g drawInBezierPath:shape angle:90];
 [[NSColor colorWithWhite:1 alpha:.45]setFill];NSRectFill(NSMakeRect(r.origin.x+2,NSMidY(r),r.size.width-4,1));
}
@end
@interface CS80Slider:NSSlider
@property AUParameter*parameter;
@end
@implementation CS80Slider
- (void)mouseDown:(NSEvent*)event{
 [self.parameter setValue:self.parameter.value originator:nil atHostTime:0 eventType:AUParameterAutomationEventTypeTouch];
 [super mouseDown:event];
 [self.parameter setValue:self.parameter.value originator:nil atHostTime:0 eventType:AUParameterAutomationEventTypeRelease];
}
@end
@interface CS80Fader:NSView
@property AUParameter*parameter;
@property CS80Slider*slider;
@property NSTextField*readout;
@property BOOL reversed,logarithmic;
@property float low,high;
- (instancetype)initWithUnit:(AUAudioUnit*)unit address:(int)address text:(NSString*)text color:(int)color frame:(NSRect)frame reversed:(BOOL)reversed;
- (void)refresh;
@end
@implementation CS80Fader
- (instancetype)initWithUnit:(AUAudioUnit*)unit address:(int)a text:(NSString*)text color:(int)color frame:(NSRect)frame reversed:(BOOL)reversed{
 if(!(self=[super initWithFrame:frame]))return nil;auto specs=cs80::makeSpecs();auto&s=specs[a];self.low=s.lo;self.high=s.hi;self.reversed=reversed;self.logarithmic=(a==cs80::SubSpeed||a==cs80::RingSpeed||a==cs80::RingAttack||a==cs80::RingDecay);self.parameter=[unit.parameterTree parameterWithAddress:a];
 CGFloat w=frame.size.width,h=frame.size.height;
 label(self,text,NSMakeRect(0,0,w,25),8.2);
 self.readout=label(self,@"",NSMakeRect(-2,h-14,w+4,13),7.8,[NSColor colorWithWhite:.62 alpha:1]);self.readout.font=[NSFont monospacedDigitSystemFontOfSize:7.5 weight:NSFontWeightRegular];
 self.slider=[[CS80Slider alloc]initWithFrame:NSMakeRect((w-31)*.5,26,31,h-42)];CS80FaderCell*cell=[CS80FaderCell new];cell.capColor=cap(color);cell.controlSize=NSControlSizeSmall;self.slider.cell=cell;self.slider.minValue=0;self.slider.maxValue=1;self.slider.continuous=YES;self.slider.target=self;self.slider.action=@selector(change:);self.slider.parameter=self.parameter;
 if(s.discrete){self.slider.numberOfTickMarks=(NSInteger)(s.hi-s.lo+1);self.slider.allowsTickMarkValuesOnly=YES;}
 self.slider.toolTip=[NSString stringWithFormat:@"%@ — %@",@(s.group.c_str()),@(s.name.c_str())];[self.slider setAccessibilityLabel:self.slider.toolTip];[self addSubview:self.slider];[self refresh];return self;
}
- (void)change:(id)sender{double n=self.slider.doubleValue;if(self.reversed)n=1-n;float value=self.logarithmic?self.low*std::pow(self.high/self.low,n):self.low+n*(self.high-self.low);[self.parameter setValue:value originator:nil atHostTime:0 eventType:AUParameterAutomationEventTypeValue];[self refresh];}
- (void)refresh{float v=self.parameter.value;double n=self.logarithmic?std::log(std::max(v,self.low)/self.low)/std::log(self.high/self.low):(v-self.low)/(self.high-self.low);if(self.reversed)n=1-n;if(!self.slider.cell.isHighlighted)self.slider.doubleValue=n;
 int a=(int)self.parameter.address;NSString*value=[NSString stringWithFormat:@"%.2g",v];
 if(a>=cs80::PanelBase){int p=(a-cs80::PanelBase)%cs80::PatchCount;if(p==cs80::IL)value=[NSString stringWithFormat:@"−%.1f",v*5];else if(p==cs80::AL)value=[NSString stringWithFormat:@"+%.1f",v*5];else if(p==cs80::PW)value=[NSString stringWithFormat:@"%.0f%%",v*100];else if(p==cs80::AmpAttack||p==cs80::FilterAttack||p==cs80::AmpDecay||p==cs80::FilterDecay||p==cs80::AmpRelease||p==cs80::FilterRelease){float seconds=cs80::timeValue(v,p==cs80::AmpAttack||p==cs80::FilterAttack);value=seconds<1?[NSString stringWithFormat:@"%.0fm",seconds*1000]:[NSString stringWithFormat:@"%.1fs",seconds];}else value=[NSString stringWithFormat:@"%.1f",v*10];}
 else if(a==cs80::FeetI||a==cs80::FeetII){NSArray*feet=@[@"16′",@"8′",@"5⅓′",@"4′",@"2⅔′",@"2′"];value=feet[std::clamp((int)std::round(v),0,5)];}
 self.readout.stringValue=value;
}
@end
@interface CS80Button:NSButton
@property NSColor*faceColor;
@end
@implementation CS80Button
- (void)drawRect:(NSRect)dirty{NSRect r=NSInsetRect(self.bounds,1,1);BOOL lit=self.state==NSControlStateValueOn;NSColor*c=self.faceColor?:[NSColor colorWithWhite:.36 alpha:1];[c setFill];NSBezierPath*p=[NSBezierPath bezierPathWithRoundedRect:r xRadius:2 yRadius:2];[p fill];[(lit?[NSColor colorWithRed:.98 green:.8 blue:.35 alpha:1]:[NSColor colorWithWhite:.65 alpha:.35])setStroke];p.lineWidth=lit?2:1;[p stroke];if(lit){[[NSColor colorWithRed:1 green:.6 blue:.13 alpha:1]setFill];NSRectFill(NSMakeRect(5,4,r.size.width-8,2));}drawText(self.title,NSInsetRect(r,2,4),8.2,[[c colorUsingColorSpace:NSColorSpace.deviceRGBColorSpace] brightnessComponent]>.56?[NSColor colorWithWhite:.09 alpha:1]:[NSColor colorWithWhite:.94 alpha:1]);}
@end
@interface CS80BoundButton:CS80Button
@property AUParameter*parameter;
@property float selectedValue;
- (void)refresh;
@end
@implementation CS80BoundButton
- (void)refresh{self.state=std::abs(self.parameter.value-self.selectedValue)<.25?NSControlStateValueOn:NSControlStateValueOff;self.needsDisplay=YES;}
@end
@interface CS80Keyboard:NSView
@property(weak) AUAudioUnit*unit;
@end
@implementation CS80Keyboard {int _mouseNote;NSMutableDictionary<NSNumber*,NSNumber*>*_typing;NSMutableSet<NSNumber*>*_down;}
- (instancetype)initWithFrame:(NSRect)r {if((self=[super initWithFrame:r])){_mouseNote=-1;_typing=[NSMutableDictionary dictionary];_down=[NSMutableSet set];}return self;}
- (BOOL)acceptsFirstResponder{return YES;}
- (BOOL)isBlack:(int)n{int p=n%12;return p==1||p==3||p==6||p==8||p==10;}
- (NSRect)rectFor:(int)n{CGFloat w=self.bounds.size.width/36.;int whites=0;for(int k=36;k<n;k++)if(![self isBlack:k])whites++;if([self isBlack:n])return NSMakeRect(whites*w-w*.32,self.bounds.size.height*.4,w*.64,self.bounds.size.height*.6);return NSMakeRect(whites*w,0,w,self.bounds.size.height);}
- (int)noteAt:(NSPoint)p{for(int n=36;n<=96;n++)if([self isBlack:n]&&NSPointInRect(p,[self rectFor:n]))return n;for(int n=36;n<=96;n++)if(![self isBlack:n]&&NSPointInRect(p,[self rectFor:n]))return n;return -1;}
- (void)drawRect:(NSRect)r{for(int pass=0;pass<2;pass++)for(int n=36;n<=96;n++){BOOL black=[self isBlack:n];if(black!=(pass==1))continue;NSRect key=[self rectFor:n];BOOL pressed=[_down containsObject:@(n)];[(pressed?[NSColor colorWithRed:.68 green:.5 blue:.25 alpha:1]:black?[NSColor colorWithWhite:.09 alpha:1]:[NSColor colorWithWhite:.93 alpha:1])setFill];NSRectFill(key);[[NSColor colorWithWhite:.2 alpha:1]setStroke];NSBezierPath*path=[NSBezierPath bezierPathWithRect:NSInsetRect(key,.5,.5)];[path stroke];}}
- (void)send:(int)status note:(int)n value:(int)v{if(n<0)return;auto b=self.unit.scheduleMIDIEventBlock;if(b){uint8_t m[3]={(uint8_t)status,(uint8_t)n,(uint8_t)v};b(AUEventSampleTimeImmediate,0,3,m);}if(status==0x90)[_down addObject:@(n)];if(status==0x80)[_down removeObject:@(n)];self.needsDisplay=YES;}
- (void)mouseDown:(NSEvent*)e{[self.window makeFirstResponder:self];NSPoint p=[self convertPoint:e.locationInWindow fromView:nil];_mouseNote=[self noteAt:p];[self send:0x90 note:_mouseNote value:std::clamp((int)(40+85*(1-p.y/self.bounds.size.height)),1,127)];}
- (void)mouseDragged:(NSEvent*)e{int n=[self noteAt:[self convertPoint:e.locationInWindow fromView:nil]];if(n!=_mouseNote){[self send:0x80 note:_mouseNote value:0];_mouseNote=n;[self send:0x90 note:n value:90];}}
- (void)mouseUp:(NSEvent*)e{[self send:0x80 note:_mouseNote value:0];_mouseNote=-1;}
- (void)pressureChangeWithEvent:(NSEvent*)e{[self send:0xa0 note:_mouseNote value:std::clamp((int)(e.pressure*127),0,127)];}
- (void)keyDown:(NSEvent*)e{if(e.isARepeat)return;NSString*k=e.charactersIgnoringModifiers.lowercaseString;NSString*map=@"awsedftgyhujkolp;";NSRange r=[map rangeOfString:k];if(k.length==1&&r.location!=NSNotFound){int n=60+(int)r.location;_typing[@(e.keyCode)]=@(n);[self send:0x90 note:n value:100];}else [super keyDown:e];}
- (void)keyUp:(NSEvent*)e{NSNumber*n=_typing[@(e.keyCode)];if(n){[self send:0x80 note:n.intValue value:0];[_typing removeObjectForKey:@(e.keyCode)];}else [super keyUp:e];}
- (void)viewWillMoveToWindow:(NSWindow*)newWindow{if(!newWindow){for(NSNumber*n in _down.copy)[self send:0x80 note:n.intValue value:0];[_typing removeAllObjects];} [super viewWillMoveToWindow:newWindow];}
@end
@interface CS80Ribbon:NSView
@property(weak)AUAudioUnit*unit;
@end
@implementation CS80Ribbon {CGFloat _origin;}
- (void)drawRect:(NSRect)r{[[NSColor colorWithRed:.24 green:.16 blue:.12 alpha:1]setFill];NSRectFill(self.bounds);NSDictionary*a=@{NSForegroundColorAttributeName:[NSColor colorWithWhite:.9 alpha:1],NSFontAttributeName:[NSFont systemFontOfSize:11]};[@"RIBBON   ·   touch anywhere, drag to bend   ·   release to return" drawAtPoint:NSMakePoint(12,4) withAttributes:a];}
- (void)mouseDown:(NSEvent*)e{_origin=[self convertPoint:e.locationInWindow fromView:nil].x;}
- (void)mouseDragged:(NSEvent*)e{float v=([self convertPoint:e.locationInWindow fromView:nil].x-_origin)/self.bounds.size.width;[self.unit.parameterTree parameterWithAddress:cs80::Ribbon].value=std::clamp(v,-1.f,1.f);}
- (void)mouseUp:(NSEvent*)e{[self.unit.parameterTree parameterWithAddress:cs80::Ribbon].value=0;}
@end

@implementation CS80ViewController {
 NSTimer*_timer;NSMutableArray<CS80Fader*>*_controls;NSMutableArray<CS80BoundButton*>*_buttons;NSMutableArray<NSPopUpButton*>*_popups;
 CS80Board*_board;NSView*_overlay;NSPopUpButton*_presetMenu;NSTextField*_presetHint;NSInteger _memorySlot;
}
- (AUAudioUnit*)createAudioUnitWithComponentDescription:(AudioComponentDescription)d error:(NSError**)error{self.unit=[[CS80AudioUnit alloc]initWithComponentDescription:d options:0 error:error];return self.unit;}
- (void)loadView{self.view=[[CS80ScaledRoot alloc]initWithFrame:NSMakeRect(0,0,1340,812)];self.preferredContentSize=NSMakeSize(1340,812);if(self.unit)[self build];}
- (void)setUnit:(AUAudioUnit*)unit{_unit=unit;if(NSThread.isMainThread){if(self.isViewLoaded)[self build];}else dispatch_async(dispatch_get_main_queue(),^{if(self.isViewLoaded)[self build];});}
- (CS80Surface*)surface:(NSString*)title rect:(NSRect)r parent:(NSView*)parent{CS80Surface*v=[[CS80Surface alloc]initWithFrame:r];v.heading=title;[parent addSubview:v];return v;}
- (void)fader:(int)a title:(NSString*)title color:(int)c rect:(NSRect)r parent:(NSView*)p reversed:(BOOL)reverse{CS80Fader*f=[[CS80Fader alloc]initWithUnit:_unit address:a text:title color:c frame:r reversed:reverse];[p addSubview:f];[_controls addObject:f];}
- (CS80Button*)button:(NSString*)title rect:(NSRect)r parent:(NSView*)p action:(SEL)action{CS80Button*b=[[CS80Button alloc]initWithFrame:r];b.title=title;b.target=self;b.action=action;b.buttonType=NSButtonTypeMomentaryPushIn;[p addSubview:b];return b;}
- (void)choice:(int)a value:(float)value title:(NSString*)title color:(NSColor*)color rect:(NSRect)r parent:(NSView*)p{
 CS80BoundButton*b=[[CS80BoundButton alloc]initWithFrame:r];b.parameter=[_unit.parameterTree parameterWithAddress:a];b.selectedValue=value;b.title=title;b.faceColor=color;b.target=self;b.action=@selector(selectValue:);[b setAccessibilityLabel:[NSString stringWithFormat:@"%@ — %@",b.parameter.displayName,title]];[p addSubview:b];[_buttons addObject:b];[b refresh];
}
- (void)toggle:(int)a title:(NSString*)title rect:(NSRect)r parent:(NSView*)p{
 CS80BoundButton*b=[[CS80BoundButton alloc]initWithFrame:r];b.parameter=[_unit.parameterTree parameterWithAddress:a];b.selectedValue=1;b.title=title;b.faceColor=[NSColor colorWithWhite:.33 alpha:1];b.target=self;b.action=@selector(toggleValue:);[b setAccessibilityLabel:b.parameter.displayName];[p addSubview:b];[_buttons addObject:b];[b refresh];
}
- (void)popup:(int)a rect:(NSRect)r parent:(NSView*)parent{
 NSPopUpButton*b=[[NSPopUpButton alloc]initWithFrame:r pullsDown:NO];auto specs=cs80::makeSpecs();for(auto&name:specs[a].choices)[b addItemWithTitle:@(name.c_str())];b.tag=a;b.font=[NSFont systemFontOfSize:10];b.target=self;b.action=@selector(popupChanged:);[b setAccessibilityLabel:[_unit.parameterTree parameterWithAddress:a].displayName];[parent addSubview:b];[_popups addObject:b];
}
- (void)programRow:(int)base title:(NSString*)title rect:(NSRect)rect parent:(NSView*)parent{
 CS80Surface*row=[self surface:@"" rect:rect parent:parent];label(row,title,NSMakeRect(7,rect.size.height-24,32,18),12,cap(4));
 CGFloat left=45,stride=(rect.size.width-left-10)/26.;
 const char*names[]={"SPEED","PWM","PW","PULSE","SAW","NOISE","HPF","RES H","LPF","RES L","IL","AL","A","D","R","VCF","SINE","A","D","S","R","LEVEL","BRILL","LEVEL","BRILL","LEVEL"};
 const int colors[]={4,1,0,0,0,5,1,2,1,2,0,0,0,0,3,5,5,0,0,0,3,5,1,5,1,5};
 const int groupStart[]={0,6,15,22};const int groupSize[]={6,9,7,4};NSArray*headings=@[@"VCO",@"VCF · IL / AL ENVELOPE",@"VCA",@"TOUCH RESPONSE"];
 for(int g=0;g<4;g++){CGFloat x=left+groupStart[g]*stride;label(row,headings[g],NSMakeRect(x,rect.size.height-21,groupSize[g]*stride-5,14),8,[NSColor colorWithWhite:.65 alpha:1]);NSBox*line=[[NSBox alloc]initWithFrame:NSMakeRect(x,rect.size.height-27,groupSize[g]*stride-7,1)];line.boxType=NSBoxSeparator;[row addSubview:line];}
 for(int p=0;p<cs80::PatchCount;p++){CGFloat x=left+p*stride;
  if(p==cs80::Square||p==cs80::Saw){[self toggle:base+p title:@"ON" rect:NSMakeRect(x+3,48,stride-6,30) parent:row];label(row,@(names[p]),NSMakeRect(x,8,stride,24),8);}
  else [self fader:base+p title:@(names[p]) color:colors[p] rect:NSMakeRect(x,5,stride,rect.size.height-44) parent:row reversed:NO];
 }
 label(row,@"INITIAL",NSMakeRect(left+22*stride,rect.size.height-36,stride*2,10),6.5,[NSColor colorWithWhite:.6 alpha:1]);label(row,@"AFTER",NSMakeRect(left+24*stride,rect.size.height-36,stride*2,10),6.5,[NSColor colorWithWhite:.6 alpha:1]);
}
- (void)build{
 if(!_unit)return;[_timer invalidate];for(NSView*v in self.view.subviews.copy)[v removeFromSuperview];_controls=[NSMutableArray array];_buttons=[NSMutableArray array];_popups=[NSMutableArray array];_overlay=nil;
 self.view.wantsLayer=YES;self.view.layer.backgroundColor=[NSColor colorWithWhite:.045 alpha:1].CGColor;self.view.appearance=[NSAppearance appearanceNamed:NSAppearanceNameDarkAqua];
 _board=[[CS80Board alloc]initWithFrame:NSMakeRect(0,0,1340,812)];[(CS80ScaledRoot*)self.view setBoard:_board];[self.view addSubview:_board];self.view.needsLayout=YES;
 label(_board,@"cs80maths",NSMakeRect(38,748,265,48),34,[NSColor colorWithWhite:.88 alpha:1],NSTextAlignmentLeft);
 label(_board,@"DIRECT GROOVES · POLYPHONIC SYNTHESIZER",NSMakeRect(42,730,240,18),9,[NSColor colorWithWhite:.5 alpha:1],NSTextAlignmentLeft);
 label(_board,@"PERFORMANCE PRESETS",NSMakeRect(328,778,300,14),8,[NSColor colorWithWhite:.6 alpha:1],NSTextAlignmentLeft);
 _presetMenu=[[NSPopUpButton alloc]initWithFrame:NSMakeRect(326,745,310,30) pullsDown:NO];[_presetMenu addItemWithTitle:@"Choose a sound…"];for(auto&name:cs80::performanceNames)[_presetMenu addItemWithTitle:@(name)];_presetMenu.target=self;_presetMenu.action=@selector(presetChanged:);_presetMenu.font=[NSFont systemFontOfSize:14 weight:NSFontWeightMedium];[_presetMenu setAccessibilityLabel:@"Performance preset"];[_board addSubview:_presetMenu];
 _presetHint=label(_board,@"Original cinematic patches · pressure adds expression",NSMakeRect(648,748,466,34),10,[NSColor colorWithWhite:.58 alpha:1],NSTextAlignmentLeft);
 [self button:@"MEMORY\nBANK" rect:NSMakeRect(1130,740,78,48) parent:_board action:@selector(openMemory:)];[self button:@"ALL\nSOUND OFF" rect:NSMakeRect(1216,740,88,48) parent:_board action:@selector(panic:)];
 [self programRow:cs80::panel(0,0) title:@"I" rect:NSMakeRect(312,568,1000,146) parent:_board];
 [self programRow:cs80::panel(1,0) title:@"II" rect:NSMakeRect(312,416,1000,146) parent:_board];
 CS80Surface*ring=[self surface:@"RING MODULATOR" rect:NSMakeRect(32,568,270,146) parent:_board];
 const int ringIDs[]={cs80::RingAttack,cs80::RingDecay,cs80::RingDepth,cs80::RingSpeed,cs80::RingMod};NSArray*ringLabels=@[@"ATTACK",@"DECAY",@"DEPTH",@"SPEED",@"MOD"];
 for(int i=0;i<5;i++)[self fader:ringIDs[i] title:ringLabels[i] color:i>=3?(i==3?4:5):0 rect:NSMakeRect(9+i*51,6,49,113) parent:ring reversed:YES];
 CS80Surface*tuning=[self surface:@"TUNING" rect:NSMakeRect(32,416,270,146) parent:_board];
 for(int i=0;i<3;i++){int ids[]={cs80::Coarse,cs80::Fine,cs80::Detune};NSArray*texts=@[@"COARSE",@"FINE",@"DETUNE II"];[self fader:ids[i] title:texts[i] color:4 rect:NSMakeRect(10+i*52,6,49,113) parent:tuning reversed:NO];}
 label(tuning,@"MEMORIES\n1 · 2 · 3 · 4",NSMakeRect(171,76,88,34),10,[NSColor colorWithWhite:.65 alpha:1]);[self button:@"OPEN\nCOVER" rect:NSMakeRect(177,25,78,42) parent:tuning action:@selector(openMemory:)];
 CS80Surface*sub=[self surface:@"SUB OSCILLATOR" rect:NSMakeRect(32,315,270,96) parent:_board];[self popup:cs80::SubFunction rect:NSMakeRect(8,18,88,28) parent:sub];
 int subIDs[]={cs80::SubSpeed,cs80::SubVCO,cs80::SubVCF,cs80::SubVCA};NSArray*subLabels=@[@"SPEED",@"VCO",@"VCF",@"VCA"];
 for(int i=0;i<4;i++)[self fader:subIDs[i] title:subLabels[i] color:i<2?4:i==2?1:5 rect:NSMakeRect(99+i*41,1,40,73) parent:sub reversed:YES];
 CS80Surface*tones=[self surface:@"TONE SELECTORS" rect:NSMakeRect(312,315,790,96) parent:_board];auto specs=cs80::makeSpecs();
 NSArray*namesI=@[@"STRING\n1",@"STRING\n3",@"BRASS\n1",@"FLUTE",@"E. PIANO",@"CLAVI\n1",@"HARPSI\n1",@"ORGAN\n1",@"GUITAR\n1",@"FUNKY\n1",@"FUNKY\n3",@"MEM\n1",@"MEM\n3",@"PANEL"];
 NSArray*namesII=@[@"STRING\n2",@"STRING\n4",@"BRASS\n2",@"BRASS\n3",@"BASS",@"CLAVI\n2",@"HARPSI\n2",@"ORGAN\n2",@"GUITAR\n2",@"FUNKY\n2",@"FUNKY\n4",@"MEM\n2",@"MEM\n4",@"PANEL"];
 for(int bank=0;bank<2;bank++){label(tones,bank?@"II":@"I",NSMakeRect(4,bank?14:46,24,22),10);for(int i=0;i<14;i++){NSColor*color=i<2?[NSColor colorWithRed:.59 green:.36 blue:.29 alpha:1]:i<5?[NSColor colorWithRed:.65 green:.67 blue:.37 alpha:1]:i<9?[NSColor colorWithRed:.64 green:.61 blue:.48 alpha:1]:i<11?[NSColor colorWithRed:.39 green:.55 blue:.55 alpha:1]:[NSColor colorWithWhite:.75 alpha:1];[self choice:bank?cs80::ToneII:cs80::ToneI value:i title:(bank?namesII:namesI)[i] color:color rect:NSMakeRect(30+i*53.9,bank?8:40,51,30) parent:tones];}}
 CS80Surface*front=[self surface:@"" rect:NSMakeRect(1110,315,202,96) parent:_board];
 [self fader:cs80::Mix title:@"MIX I / II" color:5 rect:NSMakeRect(5,1,62,91) parent:front reversed:YES];[self fader:cs80::Brilliance title:@"BRILLIANCE" color:1 rect:NSMakeRect(68,1,66,91) parent:front reversed:YES];[self fader:cs80::Resonance title:@"RESONANCE" color:2 rect:NSMakeRect(134,1,66,91) parent:front reversed:YES];
 CS80Surface*touch=[self surface:@"TOUCH RESPONSE" rect:NSMakeRect(32,167,270,142) parent:_board];
 int touchIDs[]={cs80::InitialBend,cs80::AfterSpeed,cs80::AfterVCO,cs80::AfterVCF};NSArray*touchLabels=@[@"INITIAL\nPITCHBEND",@"AFTER\nSPEED",@"AFTER\nVCO",@"AFTER\nVCF"];
 for(int i=0;i<4;i++)[self fader:touchIDs[i] title:touchLabels[i] color:i==3?1:4 rect:NSMakeRect(7+i*65,4,62,112) parent:touch reversed:YES];
 CS80Surface*feet=[self surface:@"FEET" rect:NSMakeRect(312,167,110,142) parent:_board];for(int b=0;b<2;b++)[self fader:b?cs80::FeetII:cs80::FeetI title:b?@"II":@"I" color:4 rect:NSMakeRect(5+b*52,4,49,112) parent:feet reversed:YES];
 CS80Surface*sustain=[self surface:@"SUSTAIN · PORTAMENTO / GLISSANDO" rect:NSMakeRect(430,167,218,142) parent:_board];
 [self fader:cs80::Sustain title:@"SUSTAIN" color:3 rect:NSMakeRect(6,25,52,92) parent:sustain reversed:YES];[self fader:cs80::Glide title:@"TIME" color:4 rect:NSMakeRect(111,25,49,92) parent:sustain reversed:YES];
 [self choice:cs80::SustainMode value:0 title:@"I" color:cap(0) rect:NSMakeRect(62,85,42,27) parent:sustain];[self choice:cs80::SustainMode value:1 title:@"II" color:cap(0) rect:NSMakeRect(62,53,42,27) parent:sustain];
 [self toggle:cs80::Glissando title:@"GLISS" rect:NSMakeRect(164,64,48,33) parent:sustain];[self toggle:cs80::SustainAssign title:@"FOOT SW" rect:NSMakeRect(7,3,96,23) parent:sustain];[self toggle:cs80::GlideAssign title:@"FOOT SW" rect:NSMakeRect(113,3,96,23) parent:sustain];
 CS80Surface*fx=[self surface:@"TREMOLO / CHORUS" rect:NSMakeRect(656,167,150,142) parent:_board];[self fader:cs80::EffectSpeed title:@"SPEED" color:0 rect:NSMakeRect(3,30,44,85) parent:fx reversed:NO];[self fader:cs80::EffectDepth title:@"DEPTH" color:0 rect:NSMakeRect(50,30,44,85) parent:fx reversed:NO];[self toggle:cs80::EffectOn title:@"ON" rect:NSMakeRect(100,32,44,27) parent:fx];[self choice:cs80::Tremolo value:0 title:@"CHORUS" color:cap(0) rect:NSMakeRect(4,4,69,24) parent:fx];[self choice:cs80::Tremolo value:1 title:@"TREM" color:cap(0) rect:NSMakeRect(77,4,69,24) parent:fx];
 CS80Surface*keyboard=[self surface:@"KEYBOARD CONTROL" rect:NSMakeRect(814,167,264,142) parent:_board];int kbIDs[]={cs80::LowBrilliance,cs80::HighBrilliance,cs80::LowLevel,cs80::HighLevel};NSArray*kbLabels=@[@"BRILL\nLOW",@"BRILL\nHIGH",@"LEVEL\nLOW",@"LEVEL\nHIGH"];
 for(int i=0;i<4;i++)[self fader:kbIDs[i] title:kbLabels[i] color:i<2?1:5 rect:NSMakeRect(4+i*65,4,61,112) parent:keyboard reversed:YES];
 CS80Surface*output=[self surface:@"OUTPUT · FOOT CONTROLLER" rect:NSMakeRect(1086,167,226,142) parent:_board];[self fader:cs80::Volume title:@"VOLUME" color:5 rect:NSMakeRect(8,27,60,90) parent:output reversed:NO];[self fader:cs80::Expression title:@"PEDAL" color:5 rect:NSMakeRect(72,27,60,90) parent:output reversed:NO];[self choice:cs80::ExpressionWah value:0 title:@"EXP" color:cap(5) rect:NSMakeRect(142,78,76,28) parent:output];[self choice:cs80::ExpressionWah value:1 title:@"EXP / WAH" color:cap(5) rect:NSMakeRect(142,43,76,28) parent:output];
 label(output,@"EXPRESSION",NSMakeRect(8,5,120,16),7.5,[NSColor colorWithWhite:.5 alpha:1]);
 CS80Surface*pedals=[self surface:@"FOOT SWITCH" rect:NSMakeRect(32,26,270,135) parent:_board];[self toggle:cs80::PedalConnected title:@"CONNECTED" rect:NSMakeRect(12,72,117,29) parent:pedals];[self toggle:cs80::FootSwitch title:@"DOWN" rect:NSMakeRect(139,72,117,29) parent:pedals];[self button:@"MIDI / SETTINGS" rect:NSMakeRect(12,34,244,29) parent:pedals action:@selector(openSettings:)];label(pedals,@"Click the keys, then play A W S E D F…",NSMakeRect(8,7,254,18),8,[NSColor colorWithWhite:.53 alpha:1]);
 CS80Ribbon*ribbon=[[CS80Ribbon alloc]initWithFrame:NSMakeRect(312,140,1000,21)];ribbon.unit=_unit;[_board addSubview:ribbon];CS80Keyboard*keys=[[CS80Keyboard alloc]initWithFrame:NSMakeRect(312,26,1000,106)];keys.unit=_unit;[keys setAccessibilityLabel:@"61-key performance keyboard"];[_board addSubview:keys];
 [self refreshControls];__weak CS80ViewController*weak=self;_timer=[NSTimer scheduledTimerWithTimeInterval:.08 repeats:YES block:^(NSTimer*t){CS80ViewController*s=weak;if(!s){[t invalidate];return;}[s refreshControls];}];
}
- (void)refreshControls{for(CS80Fader*c in _controls)[c refresh];for(CS80BoundButton*b in _buttons)[b refresh];for(NSPopUpButton*b in _popups){int v=(int)std::round([_unit.parameterTree parameterWithAddress:b.tag].value);[b selectItemAtIndex:std::clamp(v,0,(int)b.numberOfItems-1)];}
 NSInteger selected=_unit.currentPreset?_unit.currentPreset.number-11:-1;NSInteger menuIndex=selected>=0&&selected<cs80::PerformancePresetCount?selected+1:0;if(_presetMenu.indexOfSelectedItem!=menuIndex)[_presetMenu selectItemAtIndex:menuIndex];_presetHint.stringValue=selected>=0&&selected<cs80::PerformancePresetCount?@(cs80::performanceNotes[selected]):@"Original cinematic patches · pressure adds expression";
}
- (void)selectValue:(CS80BoundButton*)b{[b.parameter setValue:b.selectedValue originator:nil atHostTime:0 eventType:AUParameterAutomationEventTypeValue];[self refreshControls];}
- (void)toggleValue:(CS80BoundButton*)b{[b.parameter setValue:b.parameter.value>.5?0:1 originator:nil atHostTime:0 eventType:AUParameterAutomationEventTypeValue];[self refreshControls];}
- (void)popupChanged:(NSPopUpButton*)b{[_unit.parameterTree parameterWithAddress:b.tag].value=b.indexOfSelectedItem;}
- (void)presetChanged:(NSPopUpButton*)b{if(b.indexOfSelectedItem>0){NSInteger index=10+b.indexOfSelectedItem;NSArray*presets=_unit.factoryPresets;if(index<(NSInteger)presets.count)_unit.currentPreset=presets[index];}[self refreshControls];}
- (void)panic:(id)sender{CS80SendAllSoundOff(_unit);}
- (void)closeOverlay:(id)sender{if(!_overlay)return;for(CS80Fader*c in _controls.copy)if([c isDescendantOf:_overlay])[_controls removeObject:c];for(CS80BoundButton*b in _buttons.copy)if([b isDescendantOf:_overlay])[_buttons removeObject:b];for(NSPopUpButton*b in _popups.copy)if([b isDescendantOf:_overlay])[_popups removeObject:b];[_overlay removeFromSuperview];_overlay=nil;}
- (void)openMemory:(id)sender{_memorySlot=0;[self showMemory];}
- (void)memoryPage:(NSButton*)b{_memorySlot=b.tag;[self showMemory];}
- (void)showMemory{
 [self closeOverlay:nil];_overlay=[self surface:@"MEMORY BANK · PHYSICAL PANEL SETTINGS" rect:NSMakeRect(32,315,1280,399) parent:_board];
 for(int i=0;i<4;i++){CS80Button*b=[self button:[NSString stringWithFormat:@"MEMORY %d",i+1] rect:NSMakeRect(22+i*153,334,145,32) parent:_overlay action:@selector(memoryPage:)];b.tag=i;b.state=i==_memorySlot?NSControlStateValueOn:NSControlStateValueOff;}
 [self button:@"CLOSE COVER" rect:NSMakeRect(1114,334,144,32) parent:_overlay action:@selector(closeOverlay:)];
 [self programRow:cs80::memory((int)_memorySlot,0) title:[NSString stringWithFormat:@"%ld",_memorySlot+1] rect:NSMakeRect(18,97,1244,222) parent:_overlay];
 CS80Button*copy=[self button:[NSString stringWithFormat:@"COPY CHANNEL %@ PANEL",_memorySlot%2?@"II":@"I"] rect:NSMakeRect(22,42,250,34) parent:_overlay action:@selector(copyMemory:)];copy.tag=_memorySlot;
 [self button:@"SELECT THIS MEMORY" rect:NSMakeRect(284,42,220,34) parent:_overlay action:@selector(auditionMemory:)];label(_overlay,@"Memories 1 / 3 belong to I. Memories 2 / 4 belong to II. Copy leaves the source panel intact.",NSMakeRect(526,44,725,30),10,[NSColor colorWithWhite:.65 alpha:1],NSTextAlignmentLeft);[self refreshControls];
}
- (void)copyMemory:(NSButton*)b{int slot=(int)b.tag;for(int p=0;p<cs80::PatchCount;p++)[_unit.parameterTree parameterWithAddress:cs80::memory(slot,p)].value=[_unit.parameterTree parameterWithAddress:cs80::panel(slot%2,p)].value;[self refreshControls];}
- (void)auditionMemory:(id)sender{int bank=(int)_memorySlot%2;[_unit.parameterTree parameterWithAddress:bank?cs80::ToneII:cs80::ToneI].value=_memorySlot<2?11:12;[self refreshControls];}
- (void)openSettings:(id)sender{
 [self closeOverlay:nil];_overlay=[self surface:@"MIDI & MODEL SETTINGS" rect:NSMakeRect(32,315,1280,399) parent:_board];[self button:@"CLOSE" rect:NSMakeRect(1135,337,120,32) parent:_overlay action:@selector(closeOverlay:)];
 [self fader:cs80::Variation title:@"VOICE\nVARIATION" color:5 rect:NSMakeRect(35,118,100,192) parent:_overlay reversed:NO];[self fader:cs80::ExternalLevel title:@"EXTERNAL\nGAIN" color:5 rect:NSMakeRect(149,118,100,192) parent:_overlay reversed:NO];
 [self toggle:cs80::MPE title:@"MPE INPUT" rect:NSMakeRect(288,247,190,38) parent:_overlay];[self choice:cs80::OutputLow value:0 title:@"HIGH OUTPUT" color:cap(5) rect:NSMakeRect(288,189,190,38) parent:_overlay];[self choice:cs80::OutputLow value:1 title:@"LOW OUTPUT · −20 dB" color:cap(5) rect:NSMakeRect(288,141,190,38) parent:_overlay];
 NSString*text=@"APC64: leave MPE off for ordinary pad notes and polyphonic aftertouch. Enable Send Poly Aftertouch on the controller.\n\nCustom CC16–23: brilliance, resonance, mix, sub VCO, sub VCF, ring amount, ring speed, expression. CC11 is expression; CC64 connects and operates the virtual foot switch.\n\nEXT uses the Audio Unit's first audio-input channel as modulation. The standalone app does not capture external audio.\n\nThis remains an uncalibrated model. These are original Vangelis-inspired patches, not recovered settings from his recordings.";
 NSTextField*info=[NSTextField wrappingLabelWithString:text];info.frame=NSMakeRect(520,66,705,252);info.font=[NSFont systemFontOfSize:13];info.textColor=ink();[_overlay addSubview:info];label(_overlay,@"All controls, both programming panels and all four memories are saved with host state.",NSMakeRect(34,32,1200,25),11,[NSColor colorWithWhite:.57 alpha:1],NSTextAlignmentLeft);[self refreshControls];
}
- (void)dealloc{[_timer invalidate];}
@end
