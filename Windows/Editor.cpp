#include "Plugin.hpp"
#include "public.sdk/source/common/pluginview.h"
#include <windows.h>
#include <windowsx.h>
#include <memory>
#include <cstdio>
#include <cstring>
namespace dg {
static std::wstring wide(const std::string&s){int n=MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,nullptr,0);if(n<=0)return L"";std::wstring r(n,L'\0');MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,r.data(),n);r.resize(n-1);return r;}
static HBRUSH background(){static HBRUSH b=CreateSolidBrush(RGB(25,25,25));return b;}
static HINSTANCE instance(){HMODULE m=nullptr;GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(&background),&m);return m;}
static int attachedViews=0;
static HFONT font(int size=12){static HFONT normal=CreateFontW(-12,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");static HFONT title=CreateFontW(-27,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");return size>20?title:normal;}
class Editor:public CPluginView {
 struct Fader{Editor* owner;int address;HWND window=nullptr;std::wstring label;bool dragging=false;double shown=-1;};
 Controller* controller;HWND window=nullptr,groups=nullptr,presets=nullptr,banks=nullptr;
 std::vector<cs80::Spec>specs=cs80::makeSpecs();std::vector<std::unique_ptr<Fader>>faders;std::vector<std::string>groupNames;
 std::vector<HWND>globalLabels;int mode=0,group=0;static constexpr int width=1280,height=800;
 static ViewRect size(){return {0,0,width,height};}
 static LRESULT CALLBACK rootProc(HWND w,UINT m,WPARAM a,LPARAM b){Editor*e=reinterpret_cast<Editor*>(GetWindowLongPtrW(w,GWLP_USERDATA));if(m==WM_NCCREATE){e=static_cast<Editor*>(reinterpret_cast<CREATESTRUCTW*>(b)->lpCreateParams);SetWindowLongPtrW(w,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(e));}if(!e)return DefWindowProcW(w,m,a,b);return e->message(w,m,a,b);}
 static LRESULT CALLBACK faderProc(HWND w,UINT m,WPARAM a,LPARAM b){Fader*f=reinterpret_cast<Fader*>(GetWindowLongPtrW(w,GWLP_USERDATA));if(m==WM_NCCREATE){f=static_cast<Fader*>(reinterpret_cast<CREATESTRUCTW*>(b)->lpCreateParams);SetWindowLongPtrW(w,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(f));}if(!f)return DefWindowProcW(w,m,a,b);auto*e=f->owner;auto set=[&](double v){e->controller->edit(f->address,std::clamp(v,0.,1.));InvalidateRect(w,nullptr,FALSE);};switch(m){
 case WM_PAINT:{PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);RECT r;GetClientRect(w,&r);FillRect(dc,&r,background());SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(216,216,216));SelectObject(dc,font());RECT label={0,0,r.right,32};DrawTextW(dc,f->label.c_str(),-1,&label,DT_CENTER|DT_WORDBREAK);double v=e->controller->getParamNormalized(f->address);int x=r.right/2,y=47+int((1-v)*80);HPEN pen=CreatePen(PS_SOLID,3,RGB(6,6,6));auto old=SelectObject(dc,pen);MoveToEx(dc,x,43,nullptr);LineTo(dc,x,132);SelectObject(dc,old);DeleteObject(pen);pen=CreatePen(PS_SOLID,1,RGB(72,72,72));old=SelectObject(dc,pen);for(int i=0;i<11;i++){int yy=45+i*8;MoveToEx(dc,x-12,yy,nullptr);LineTo(dc,x-7,yy);MoveToEx(dc,x+7,yy,nullptr);LineTo(dc,x+12,yy);}SelectObject(dc,old);DeleteObject(pen);int p=f->address>=cs80::PanelBase?(f->address-cs80::PanelBase)%cs80::PatchCount:-1;COLORREF color=(p==cs80::HPF||p==cs80::LPF||p==cs80::AfterBrilliance||p==cs80::PWMDepth)?RGB(100,160,108):(p==cs80::ResH||p==cs80::ResL)?RGB(190,83,73):(p==cs80::AmpRelease||p==cs80::FilterRelease)?RGB(200,174,65):RGB(182,182,182);RECT cap={x-11,y-5,x+11,y+6};HBRUSH brush=CreateSolidBrush(color);FillRect(dc,&cap,brush);DeleteObject(brush);char text[32];std::snprintf(text,sizeof(text),"%.3g",plain(e->specs[f->address],v));auto ws=wide(text);RECT value={0,141,r.right,159};DrawTextW(dc,ws.c_str(),-1,&value,DT_CENTER);EndPaint(w,&ps);return 0;}
 case WM_LBUTTONDOWN:SetFocus(w);SetCapture(w);f->dragging=true;set(1.-(GET_Y_LPARAM(b)-47)/80.);return 0;
 case WM_MOUSEMOVE:if(f->dragging)set(1.-(GET_Y_LPARAM(b)-47)/80.);return 0;
 case WM_LBUTTONUP:if(f->dragging){f->dragging=false;ReleaseCapture();if(f->address==cs80::Ribbon)set(.5);}return 0;
 case WM_CAPTURECHANGED:f->dragging=false;return 0;
 case WM_KEYDOWN:{double v=e->controller->getParamNormalized(f->address);if(a==VK_UP||a==VK_RIGHT)set(v+.01);if(a==VK_DOWN||a==VK_LEFT)set(v-.01);if(a==VK_HOME)set(0);if(a==VK_END)set(1);return 0;}
 case WM_MOUSEWHEEL:set(e->controller->getParamNormalized(f->address)+(GET_WHEEL_DELTA_WPARAM(a)>0?.01:-.01));return 0;
 }return DefWindowProcW(w,m,a,b);}
 HWND control(const wchar_t*cls,const std::wstring&text,int x,int y,int w,int h,int id=0,DWORD extra=0){HWND child=CreateWindowW(cls,text.c_str(),WS_CHILD|WS_VISIBLE|extra,x,y,w,h,window,reinterpret_cast<HMENU>(INT_PTR(id)),instance(),nullptr);SendMessageW(child,WM_SETFONT,reinterpret_cast<WPARAM>(font()),TRUE);return child;}
 void addFader(int address,int x,int y,int w,const std::wstring&label){auto f=std::make_unique<Fader>();f->owner=this;f->address=address;f->label=label;f->window=CreateWindowW(L"DGcs80Fader",wide(specs[address].group+" "+specs[address].name).c_str(),WS_CHILD|WS_VISIBLE|WS_TABSTOP,x,y,w,166,window,nullptr,instance(),f.get());faders.push_back(std::move(f));}
 void rebuild(){for(auto&f:faders)DestroyWindow(f->window);faders.clear();for(auto w:globalLabels)DestroyWindow(w);globalLabels.clear();
  static const wchar_t*names[]={L"PWM\nSPEED",L"PWM",L"PW",L"PULSE",L"SAW",L"NOISE",L"HPF",L"RES H",L"LPF",L"RES L",L"IL",L"AL",L"VCF A",L"VCF D",L"VCF R",L"VCF\nLEVEL",L"SINE",L"VCA A",L"VCA D",L"SUSTAIN",L"VCA R",L"LEVEL",L"INITIAL\nBRILL",L"INITIAL\nLEVEL",L"AFTER\nBRILL",L"AFTER\nLEVEL"};
  for(int bank=0;bank<2;bank++){int base=mode==0?cs80::panel(bank,0):cs80::memory((mode-1)*2+bank,0);for(int p=0;p<cs80::PatchCount;p++)addFader(base+p,30+p*47,115+bank*205,46,names[p]);}
  int j=0;for(int i=0;i<cs80::GlobalCount;i++)if(specs[i].group==groupNames[group]){auto&s=specs[i];int x=30+j*103;if(!s.choices.empty()){auto label=control(L"STATIC",wide(s.name),x,600,100,28);auto combo=control(L"COMBOBOX",L"",x,634,100,250,200+i,CBS_DROPDOWNLIST|WS_VSCROLL|WS_TABSTOP);for(auto&c:s.choices)SendMessageW(combo,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(wide(c).c_str()));SendMessageW(combo,CB_SETCURSEL,int(std::round(plain(s,controller->getParamNormalized(i)))),0);globalLabels.push_back(label);globalLabels.push_back(combo);}else addFader(i,x,590,98,wide(s.name));j++;}
  InvalidateRect(window,nullptr,TRUE);
 }
 void createControls(){control(L"STATIC",L"cs80maths · Direct Grooves",24,13,360,35);control(L"STATIC",L"PERFORMANCE PRESETS",430,9,300,20);presets=control(L"COMBOBOX",L"",430,33,280,280,1,CBS_DROPDOWNLIST|WS_TABSTOP);SendMessageW(presets,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"Custom / current settings"));for(auto&name:cs80::performanceNames)SendMessageW(presets,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(wide(name).c_str()));SendMessageW(presets,CB_SETCURSEL,controller->selectedPreset(),0);
  banks=control(L"COMBOBOX",L"",750,33,250,160,2,CBS_DROPDOWNLIST|WS_TABSTOP);for(auto name:{L"Edit main panels I / II",L"Edit memories 1 / 2",L"Edit memories 3 / 4"})SendMessageW(banks,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(name));SendMessageW(banks,CB_SETCURSEL,0,0);control(L"BUTTON",L"All sound off",1050,28,180,32,3,WS_TABSTOP);
  control(L"STATIC",L"CHANNEL I / odd memory",30,83,600,24);control(L"STATIC",L"CHANNEL II / even memory",30,288,600,24);control(L"STATIC",L"PERFORMANCE CONTROLS",30,535,260,24);groups=control(L"COMBOBOX",L"",320,530,300,280,4,CBS_DROPDOWNLIST|WS_TABSTOP);for(auto&name:groupNames)SendMessageW(groups,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(wide(name).c_str()));SendMessageW(groups,CB_SETCURSEL,0,0);control(L"STATIC",L"Select Panel or the corresponding Memory in Tone selectors to hear edits. Play using your host's MIDI input.",30,768,1210,25);rebuild();SetTimer(window,1,33,nullptr);
 }
 LRESULT message(HWND w,UINT m,WPARAM a,LPARAM b){switch(m){case WM_COMMAND:{int id=LOWORD(a);if(id==3){for(int ch=0;ch<16;ch++)controller->edit(midiID(ch,120),1);return 0;}if(HIWORD(a)==CBN_SELCHANGE){if(id==1)controller->choosePreset(int(SendMessageW(presets,CB_GETCURSEL,0,0)));else if(id==2){mode=int(SendMessageW(banks,CB_GETCURSEL,0,0));rebuild();}else if(id==4){group=int(SendMessageW(groups,CB_GETCURSEL,0,0));rebuild();}else if(id>=200&&id<200+cs80::GlobalCount){int address=id-200;auto&s=specs[address];controller->edit(address,normalized(s,float(SendMessageW(reinterpret_cast<HWND>(b),CB_GETCURSEL,0,0))));}}return 0;}
 case WM_TIMER:for(auto&f:faders){double v=controller->getParamNormalized(f->address);if(v!=f->shown){f->shown=v;InvalidateRect(f->window,nullptr,FALSE);}}for(auto child:globalLabels){int id=GetDlgCtrlID(child);if(id>=200){int i=id-200;int v=int(std::round(plain(specs[i],controller->getParamNormalized(i))));if(SendMessageW(child,CB_GETCURSEL,0,0)!=v)SendMessageW(child,CB_SETCURSEL,v,0);}}SendMessageW(presets,CB_SETCURSEL,controller->selectedPreset(),0);return 0;
 case WM_CTLCOLORSTATIC:SetTextColor(reinterpret_cast<HDC>(a),RGB(222,222,222));SetBkColor(reinterpret_cast<HDC>(a),RGB(25,25,25));return reinterpret_cast<LRESULT>(background());
 case WM_ERASEBKGND:{RECT r;GetClientRect(w,&r);FillRect(reinterpret_cast<HDC>(a),&r,background());return 1;}
 case WM_DESTROY:KillTimer(w,1);return 0;
 }return DefWindowProcW(w,m,a,b);}
public:
 explicit Editor(Controller*c):CPluginView(),controller(c){setRect(size());controller->addRef();for(int i=0;i<cs80::GlobalCount;i++)if(std::find(groupNames.begin(),groupNames.end(),specs[i].group)==groupNames.end())groupNames.push_back(specs[i].group);}
 ~Editor()override{removed();controller->release();}
 tresult PLUGIN_API isPlatformTypeSupported(FIDString type)override{return type&&std::strcmp(type,kPlatformTypeHWND)==0?kResultTrue:kResultFalse;}
 tresult PLUGIN_API attached(void*parent,FIDString type)override{if(!parent||window||isPlatformTypeSupported(type)!=kResultTrue)return kResultFalse;
  WNDCLASSW wc{};wc.hInstance=instance();wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);wc.lpszClassName=L"DGcs80Editor";wc.lpfnWndProc=rootProc;RegisterClassW(&wc);wc.lpszClassName=L"DGcs80Fader";wc.lpfnWndProc=faderProc;RegisterClassW(&wc);
  window=CreateWindowW(L"DGcs80Editor",L"cs80maths",WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,0,0,width,height,static_cast<HWND>(parent),nullptr,instance(),this);if(!window)return kResultFalse;systemWindow=parent;++attachedViews;createControls();return kResultOk;}
 tresult PLUGIN_API removed()override{if(window){DestroyWindow(window);window=nullptr;if(--attachedViews==0){UnregisterClassW(L"DGcs80Fader",instance());UnregisterClassW(L"DGcs80Editor",instance());}}faders.clear();globalLabels.clear();systemWindow=nullptr;return kResultOk;}
};
IPlugView* PLUGIN_API Controller::createView(FIDString name){return name&&std::strcmp(name,ViewType::kEditor)==0?new Editor(this):nullptr;}
}
