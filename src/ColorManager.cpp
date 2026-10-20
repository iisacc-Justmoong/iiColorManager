#include <iiColorManager/ColorManager.h>
#include <algorithm>
#include <cmath>
#include <numbers>

namespace iiColorManager {
namespace {
double bounded(double v,double lo=0,double hi=1) noexcept { return std::isfinite(v)?std::clamp(v,lo,hi):lo; }
double hue(double v) noexcept { if(!std::isfinite(v))return 0;v=std::fmod(v,360.);return v<0?v+360:v; }
bool finite(Rgba c) noexcept { return std::isfinite(c.r)&&std::isfinite(c.g)&&std::isfinite(c.b)&&std::isfinite(c.a); }
Rgba normalize(Rgba c) noexcept { return {bounded(c.r),bounded(c.g),bounded(c.b),bounded(c.a)}; }
double linear(double c) noexcept { return c<=.04045?c/12.92:std::pow((c+.055)/1.055,2.4); }
double encoded(double c) noexcept { return c<=.0031308?12.92*c:1.055*std::pow(c,1/2.4)-.055; }
using Triple=std::array<double,3>;
using Matrix=std::array<Triple,3>;
Triple multiply(const Matrix& m,Triple v) noexcept {
    return {m[0][0]*v[0]+m[0][1]*v[1]+m[0][2]*v[2],m[1][0]*v[0]+m[1][1]*v[1]+m[1][2]*v[2],m[2][0]*v[0]+m[2][1]*v[1]+m[2][2]*v[2]};
}
constexpr Matrix rgbToXyz{{{.41239079926595934,.35758433938387796,.1804807884018343},
    {.21263900587151027,.7151686787677559,.07219231536073371},
    {.01933081871559185,.11919477979462599,.9505321522496607}}};
constexpr Matrix xyzToRgb{{{3.2409699419045226,-1.537383177570094,-.4986107602930034},
    {-.9692436362808796,1.8759675015077202,.04155505740717559},
    {.05563007969699366,-.20397695888897652,1.0569715142428786}}};
constexpr Matrix d65ToD50{{{1.0479297925449969,.022946870601609652,-.05019226628920524},
    {.02962780877005599,.9904344267538799,-.017073799063418826},
    {-.009243040646204504,.015055191490298152,.7518742814281371}}};
constexpr Matrix d50ToD65{{{.955473421488075,-.02309845494876471,.06325924320057072},
    {-.0283697093338637,1.0099953980813041,.021041441191917323},
    {.012314014864481998,-.020507649298898964,1.330365926242124}}};
constexpr Triple white{.3457/.3585,1,(1-.3457-.3585)/.3585};
constexpr std::array hsbChannels{Channel::Hue,Channel::Saturation,Channel::Brightness};
constexpr std::array rgbChannels{Channel::Red,Channel::Green,Channel::Blue};
constexpr std::array grayChannels{Channel::Gray};
constexpr std::array cmykChannels{Channel::Cyan,Channel::Magenta,Channel::Yellow,Channel::Black};
constexpr std::array labChannels{Channel::Lightness,Channel::LabA,Channel::LabB};
constexpr std::array webChannels{Channel::WebRed,Channel::WebGreen,Channel::WebBlue};
constexpr std::array<ModeInfo,9> modeTable{{
    {Mode::HueCube,"hue-cube","Hue Cube",hsbChannels,true},
    {Mode::BrightnessCube,"brightness-cube","Brightness Cube",hsbChannels,true},
    {Mode::ColorWheel,"color-wheel","Color Wheel",hsbChannels,true},
    {Mode::GrayscaleSlider,"grayscale-slider","Grayscale Slider",grayChannels,false},
    {Mode::RgbSliders,"rgb-sliders","RGB Sliders",rgbChannels,false},
    {Mode::HsbSliders,"hsb-sliders","HSB Sliders",hsbChannels,false},
    {Mode::CmykSliders,"cmyk-sliders","CMYK Sliders",cmykChannels,false},
    {Mode::LabSliders,"lab-sliders","Lab Sliders",labChannels,false},
    {Mode::WebColorSliders,"web-color-sliders","Web Color Sliders",webChannels,false}}};
constexpr std::array<ChannelInfo,18> channelTable{{
    {Channel::Hue,"H","Hue","°",0,360,1},{Channel::Saturation,"S","Saturation","%",0,100,1},
    {Channel::Brightness,"B","Brightness","%",0,100,1},{Channel::Red,"R","Red","",0,255,1},
    {Channel::Green,"G","Green","",0,255,1},{Channel::Blue,"B","Blue","",0,255,1},
    {Channel::Cyan,"C","Cyan","%",0,100,1},{Channel::Magenta,"M","Magenta","%",0,100,1},
    {Channel::Yellow,"Y","Yellow","%",0,100,1},{Channel::Black,"K","Black","%",0,100,1},
    {Channel::Alpha,"A","Alpha","%",0,100,1},{Channel::Gray,"K","Grayscale","%",0,100,1},
    {Channel::Lightness,"L","Lightness","",0,100,1},{Channel::LabA,"a","Lab a","",-128,127,1},
    {Channel::LabB,"b","Lab b","",-128,127,1},{Channel::WebRed,"R","Web red","",0,255,51},
    {Channel::WebGreen,"G","Web green","",0,255,51},{Channel::WebBlue,"B","Web blue","",0,255,51}}};
bool valid(Surface s) noexcept { return int(s)>=0&&int(s)<=int(Surface::ChannelRamp); }
}

std::span<const ModeInfo> modes() noexcept { return modeTable; }
const ModeInfo* modeInfo(Mode m) noexcept { return int(m)>=0&&int(m)<int(modeTable.size())?&modeTable[std::size_t(m)]:nullptr; }
const ChannelInfo* channelInfo(Channel c) noexcept { return int(c)>=0&&int(c)<int(channelTable.size())?&channelTable[std::size_t(c)]:nullptr; }
Hsb toHsb(Rgba input) noexcept {
    const auto c=normalize(input);const double mx=std::max({c.r,c.g,c.b}),mn=std::min({c.r,c.g,c.b}),d=mx-mn;
    double h=0;if(d>0) { if(mx==c.r)h=(c.g-c.b)/d;else if(mx==c.g)h=(c.b-c.r)/d+2;else h=(c.r-c.g)/d+4; }
    return {hue(h*60),mx>0?d/mx*100:0,mx*100};
}
Rgba fromHsb(Hsb input,double alpha) noexcept {
    const double h=hue(input.h)/60,s=bounded(input.s,0,100)/100,v=bounded(input.b,0,100)/100;
    const double c=v*s,x=c*(1-std::abs(std::fmod(h,2.)-1)),m=v-c;
    Triple rgb{};if(h<1)rgb={c,x,0};else if(h<2)rgb={x,c,0};else if(h<3)rgb={0,c,x};else if(h<4)rgb={0,x,c};else if(h<5)rgb={x,0,c};else rgb={c,0,x};
    return {rgb[0]+m,rgb[1]+m,rgb[2]+m,bounded(alpha)};
}
Cmyk toCmyk(Rgba input) noexcept {
    const auto c=normalize(input);const auto max=std::max({c.r,c.g,c.b});
    if(max==0)return {0,0,0,100};return {(1-c.r/max)*100,(1-c.g/max)*100,(1-c.b/max)*100,(1-max)*100};
}
Rgba fromCmyk(Cmyk c,double alpha) noexcept {
    const auto white=1-bounded(c.k,0,100)/100;return {(1-bounded(c.c,0,100)/100)*white,(1-bounded(c.m,0,100)/100)*white,(1-bounded(c.y,0,100)/100)*white,bounded(alpha)};
}
Lab toLab(Rgba input) noexcept {
    const auto c=normalize(input);auto xyz=multiply(d65ToD50,multiply(rgbToXyz,{linear(c.r),linear(c.g),linear(c.b)}));
    for(std::size_t i=0;i<3;i++) { const auto n=xyz[i]/white[i];xyz[i]=n>216./24389?std::cbrt(n):(24389./27*n+16)/116; }
    return {116*xyz[1]-16,500*(xyz[0]-xyz[1]),200*(xyz[1]-xyz[2])};
}
GamutColor fromLab(Lab c,double alpha) noexcept {
    const double fy=(bounded(c.l,0,100)+16)/116,fx=fy+bounded(c.a,-128,127)/500,fz=fy-bounded(c.b,-128,127)/200;
    Triple xyz{fx,fy,fz};for(std::size_t i=0;i<3;i++) { auto f=xyz[i],cube=f*f*f;xyz[i]=white[i]*(cube>216./24389?cube:(116*f-16)/(24389./27)); }
    const auto rgb=multiply(xyzToRgb,multiply(d50ToD65,xyz));const bool out=std::any_of(rgb.begin(),rgb.end(),[](double n){return n< -1e-7||n>1+1e-7;});
    return {normalize({encoded(rgb[0]),encoded(rgb[1]),encoded(rgb[2]),alpha}),out};
}
Rgba nearestWebSafe(Rgba c) noexcept { c=normalize(c);return {std::round(c.r*5)/5,std::round(c.g*5)/5,std::round(c.b*5)/5,c.a}; }
bool isWebSafe(Rgba c) noexcept { if(!finite(c))return false;const auto w=nearestWebSafe(c);return std::abs(c.r-w.r)<1e-7&&std::abs(c.g-w.g)<1e-7&&std::abs(c.b-w.b)<1e-7; }
std::optional<Rgba> parseHex(std::string_view text,double alpha) noexcept {
    while(!text.empty()&&(text.front()==' '||text.front()=='\t'||text.front()=='\n'||text.front()=='\r'))text.remove_prefix(1);
    while(!text.empty()&&(text.back()==' '||text.back()=='\t'||text.back()=='\n'||text.back()=='\r'))text.remove_suffix(1);
    if(!text.empty()&&text.front()=='#')text.remove_prefix(1);
    const auto size=text.size();if(size!=3&&size!=4&&size!=6&&size!=8)return {};
    std::array<int,8> digits{};for(std::size_t i=0;i<size;i++) { const char c=text[i];digits[i]=c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:-1;if(digits[i]<0)return {}; }
    const bool shortForm=size<5;auto channel=[&](std::size_t i){return (shortForm?digits[i]*17:digits[i*2]*16+digits[i*2+1])/255.;};
    return Rgba{channel(0),channel(1),channel(2),(size==4||size==8)?channel(3):bounded(alpha)};
}
std::string toHex(Rgba input,bool alpha) {
    constexpr char hex[]="0123456789ABCDEF";const auto c=normalize(input);std::string text="#";for(const double n:{c.r,c.g,c.b,c.a}) { const auto b=unsigned(std::lround(n*255));text+=hex[b>>4];text+=hex[b&15];if(text.size()==7&&!alpha)break; }return text;
}
Picker::Picker() noexcept { syncRgb();baseline_=state_; }
bool Picker::setMode(Mode mode) noexcept { if(!modeInfo(mode))return false;mode_=mode;return true; }
void Picker::syncRgb() noexcept {
    auto h=toHsb(state_.rgb);if(h.b==0) { h.h=state_.hsb.h;h.s=state_.hsb.s; }else if(h.s==0)h.h=state_.hsb.h;
    state_.hsb=h;state_.cmyk=toCmyk(state_.rgb);state_.lab=toLab(state_.rgb);state_.outOfGamut=false;
}
bool Picker::begin(Rgba c) noexcept { if(!editColor(c))return false;baseline_=state_;return true; }
bool Picker::editColor(Rgba c) noexcept { if(!finite(c))return false;state_.rgb=normalize(c);syncRgb();return true; }
bool Picker::editHex(std::string_view text) noexcept { const auto c=parseHex(text,state_.rgb.a);return c&&editColor(*c); }
double Picker::channelValue(Channel c) const noexcept {
    const auto& s=state_;switch(c) {
    case Channel::Hue:return s.hsb.h;case Channel::Saturation:return s.hsb.s;case Channel::Brightness:return s.hsb.b;
    case Channel::Red:case Channel::WebRed:return s.rgb.r*255;case Channel::Green:case Channel::WebGreen:return s.rgb.g*255;case Channel::Blue:case Channel::WebBlue:return s.rgb.b*255;
    case Channel::Cyan:return s.cmyk.c;case Channel::Magenta:return s.cmyk.m;case Channel::Yellow:return s.cmyk.y;case Channel::Black:return s.cmyk.k;
    case Channel::Lightness:return s.lab.l;case Channel::LabA:return s.lab.a;case Channel::LabB:return s.lab.b;case Channel::Alpha:return s.rgb.a*100;
    case Channel::Gray:return (1-encoded(.2126*linear(s.rgb.r)+.7152*linear(s.rgb.g)+.0722*linear(s.rgb.b)))*100;
    }return 0;
}
bool Picker::edit(Channel c,double value) noexcept {
    const auto info=channelInfo(c);if(!info||!std::isfinite(value))return false;
    value=c==Channel::Hue?hue(value):std::clamp(value,info->minimum,info->maximum);
    if(c==Channel::Alpha) { state_.rgb.a=value/100;return true; }
    if(c==Channel::Hue||c==Channel::Saturation||c==Channel::Brightness) {
        if(c==Channel::Hue)state_.hsb.h=value;else if(c==Channel::Saturation)state_.hsb.s=value;else state_.hsb.b=value;
        state_.rgb=fromHsb(state_.hsb,state_.rgb.a);state_.cmyk=toCmyk(state_.rgb);state_.lab=toLab(state_.rgb);state_.outOfGamut=false;return true;
    }
    if(c>=Channel::Cyan&&c<=Channel::Black) {
        if(c==Channel::Cyan)state_.cmyk.c=value;else if(c==Channel::Magenta)state_.cmyk.m=value;else if(c==Channel::Yellow)state_.cmyk.y=value;else state_.cmyk.k=value;
        const auto requested=state_.cmyk;state_.rgb=fromCmyk(requested,state_.rgb.a);syncRgb();state_.cmyk=requested;return true;
    }
    if(c==Channel::Lightness||c==Channel::LabA||c==Channel::LabB) {
        if(c==Channel::Lightness)state_.lab.l=value;else if(c==Channel::LabA)state_.lab.a=value;else state_.lab.b=value;
        const auto requested=state_.lab;const auto converted=fromLab(requested,state_.rgb.a);state_.rgb=converted.color;syncRgb();state_.lab=requested;state_.outOfGamut=converted.outOfGamut;return true;
    }
    auto rgb=state_.rgb;if(c==Channel::Red||c==Channel::WebRed)rgb.r=value/255;else if(c==Channel::Green||c==Channel::WebGreen)rgb.g=value/255;else if(c==Channel::Blue||c==Channel::WebBlue)rgb.b=value/255;
    else if(c==Channel::Gray)rgb.r=rgb.g=rgb.b=1-value/100;
    if(c>=Channel::WebRed&&c<=Channel::WebBlue)rgb=nearestWebSafe(rgb);
    return editColor(rgb);
}
bool Picker::editSurface(Surface s,double x,double y,Channel c) noexcept {
    if(!valid(s)||!std::isfinite(x)||!std::isfinite(y))return false;x=bounded(x);y=bounded(y);
    if(s==Surface::ChannelRamp) { const auto i=channelInfo(c);return i&&edit(c,i->minimum+x*(i->maximum-i->minimum)); }
    if(s==Surface::HueRail)return edit(Channel::Hue,(1-y)*360);
    if(s==Surface::BrightnessRail)return edit(Channel::Brightness,(1-y)*100);
    auto h=state_.hsb;
    if(s==Surface::HueCube) { h.s=x*100;h.b=(1-y)*100; }
    else if(s==Surface::BrightnessCube) { h.h=hue(x*360);h.s=(1-y)*100; }
    else if(s==Surface::Spectrum) { h={hue(x*360),100,100}; }
    else { const auto dx=x*2-1,dy=1-y*2,r=std::hypot(dx,dy);if(r>1e-12)h.h=hue(std::atan2(dy,dx)*180/std::numbers::pi);h.s=std::min(1.,r)*100; }
    state_.hsb=h;state_.rgb=fromHsb(h,state_.rgb.a);state_.cmyk=toCmyk(state_.rgb);state_.lab=toLab(state_.rgb);state_.outOfGamut=false;return true;
}
Rgba Picker::surfaceColor(Surface s,double x,double y,Channel c) const noexcept {
    if(!valid(s)||!std::isfinite(x)||!std::isfinite(y))return {0,0,0,0};
    x=bounded(x);y=bounded(y);const auto h=state_.hsb;
    if(s==Surface::HueCube)return fromHsb({h.h,x*100,(1-y)*100});
    if(s==Surface::BrightnessCube)return fromHsb({x*360,(1-y)*100,h.b});
    if(s==Surface::HueRail)return fromHsb({(1-y)*360,100,100});
    if(s==Surface::BrightnessRail)return fromHsb({h.h,h.s,(1-y)*100});
    if(s==Surface::Spectrum)return fromHsb({x*360,100,100});
    if(s==Surface::ColorWheel) {
        const double dx=x*2-1,dy=1-y*2,r=std::hypot(dx,dy);if(r>1)return {0,0,0,0};
        return fromHsb({r>1e-12?std::atan2(dy,dx)*180/std::numbers::pi:h.h,r*100,100});
    }
    Picker copy=*this;if(!copy.editSurface(s,x,y,c))return {0,0,0,0};auto color=copy.color();if(c!=Channel::Alpha)color.a=1;return color;
}
Point Picker::position(Surface s,Channel c) const noexcept {
    const auto h=state_.hsb;switch(s) {
    case Surface::HueCube:return {h.s/100,1-h.b/100};case Surface::BrightnessCube:return {h.h/360,1-h.s/100};
    case Surface::HueRail:return {.5,1-h.h/360};case Surface::BrightnessRail:return {.5,1-h.b/100};
    case Surface::ColorWheel:{const double angle=h.h*std::numbers::pi/180;return {.5+.5*h.s/100*std::cos(angle),.5-.5*h.s/100*std::sin(angle)};}
    case Surface::Spectrum:return {h.h/360,.5};case Surface::ChannelRamp:{const auto i=channelInfo(c);return i?Point{(channelValue(c)-i->minimum)/(i->maximum-i->minimum),.5}:Point{};}
    }return {};
}
std::optional<SurfaceImage> renderSurface(const Picker& p,Surface s,std::uint32_t w,std::uint32_t h,Channel c) {
    if(!valid(s)||!w||!h||std::uint64_t(w)*h>16'000'000||(s==Surface::ChannelRamp&&!channelInfo(c)))return {};
    SurfaceImage result{w,h,std::vector<std::uint8_t>(std::size_t(w)*h*4)};
    const bool vertical=s==Surface::HueRail||s==Surface::BrightnessRail;
    for(std::uint32_t y=0;y<h;y++) {
      if(y&&(s==Surface::ChannelRamp||s==Surface::Spectrum)) { std::copy_n(result.rgba.begin(),std::size_t(w)*4,result.rgba.begin()+std::size_t(y)*w*4);continue; }
      for(std::uint32_t x=0;x<(vertical?1:w);x++) {const auto color=p.surfaceColor(s,w>1?double(x)/(w-1):.5,h>1?double(y)/(h-1):.5,c);const auto i=(std::size_t(y)*w+x)*4;
        result.rgba[i]=std::uint8_t(std::lround(color.r*255));result.rgba[i+1]=std::uint8_t(std::lround(color.g*255));result.rgba[i+2]=std::uint8_t(std::lround(color.b*255));result.rgba[i+3]=std::uint8_t(std::lround(color.a*255));}
      if(vertical) {
          const auto row=std::size_t(y)*w*4;
          for(std::uint32_t x=1;x<w;++x)
              std::copy_n(result.rgba.begin()+row,4,result.rgba.begin()+row+std::size_t(x)*4);
      }
    }
    return result;
}
} // namespace iiColorManager
