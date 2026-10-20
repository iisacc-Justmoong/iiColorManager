#include <iiColorManager/ColorManager.h>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace CM=iiColorManager;
void require(bool value,const char* message) { if(!value) throw std::runtime_error(message); }
void near(double actual,double expected,double epsilon=1e-7) { if(!std::isfinite(actual)||std::abs(actual-expected)>epsilon) {
    std::cerr<<actual<<" != "<<expected<<'\n'; throw std::runtime_error("Numeric reference mismatch"); } }
void same(CM::Rgba a,CM::Rgba b,double eps=1e-7) { near(a.r,b.r,eps);near(a.g,b.g,eps);near(a.b,b.b,eps);near(a.a,b.a,eps); }
void conversions() {
    same(CM::fromHsb({0,100,100},.25),{1,0,0,.25});
    same(CM::fromHsb({120,100,100}),{0,1,0,1});
    same(CM::fromHsb({240,100,100}),{0,0,1,1});
    same(CM::fromHsb({-60,100,100}),{1,0,1,1});
    same(CM::fromCmyk({0,100,100,0},.3),{1,0,0,.3});
    same(CM::fromCmyk({0,0,0,100}),{0,0,0,1});
    for(int r=0;r<=10;r++) for(int g=0;g<=10;g++) for(int b=0;b<=10;b++) {
        CM::Rgba c{r/10.,g/10.,b/10.,.37};same(CM::fromHsb(CM::toHsb(c),c.a),c);same(CM::fromCmyk(CM::toCmyk(c),c.a),c);
    }
    same(*CM::parseHex(" #f08 ",.25),{1,0,136/255.,.25});
    same(*CM::parseHex("#ff000080"),{1,0,0,128/255.});
    require(CM::toHex({1,0,.5,.25},true)=="#FF008040","Hex byte order");
    require(!CM::parseHex("#xx0000")&&!CM::parseHex("#12345"),"Invalid hex accepted");
}
void lab() {
    auto white=CM::toLab({1,1,1,1});near(white.l,100,1e-5);near(white.a,0,1e-4);near(white.b,0,1e-4);
    auto red=CM::toLab({1,0,0,1});near(red.l,54.29054,.002);near(red.a,80.80492,.002);near(red.b,69.89099,.002);
    for(int r=0;r<=8;r++) for(int g=0;g<=8;g++) for(int b=0;b<=8;b++) {
        CM::Rgba c{r/8.,g/8.,b/8.,.41};same(CM::fromLab(CM::toLab(c),c.a).color,c,3e-6);
    }
    require(CM::fromLab({50,127,127}).outOfGamut,"Lab gamut warning missing");
}
void state() {
    CM::Picker separation;require(separation.begin({1,1,1,1}),"CMYK begin failed");
    for(auto channel:{CM::Channel::Cyan,CM::Channel::Magenta,CM::Channel::Yellow})require(separation.edit(channel,50),"CMYK edit failed");
    near(separation.cmyk().c,50);near(separation.cmyk().m,50);near(separation.cmyk().y,50);near(separation.cmyk().k,0);
    require(separation.edit(CM::Channel::Black,20),"Black edit failed");same(separation.color(),{.4,.4,.4,1});near(separation.cmyk().k,20);
    CM::Picker p;require(p.begin({.2,.4,.6,.3}),"Begin failed");const auto original=p.color();
    require(p.edit(CM::Channel::Hue,250)&&p.edit(CM::Channel::Saturation,80)&&p.edit(CM::Channel::Brightness,0),"HSB edit failed");
    near(p.hsb().h,250);near(p.hsb().s,80);require(p.edit(CM::Channel::Brightness,100),"Brightness edit failed");near(p.hsb().h,250);near(p.hsb().s,80);near(p.color().a,.3);
    p.cancel();same(p.color(),original);
    require(p.edit(CM::Channel::Black,50),"CMYK edit failed");near(p.cmyk().k,50);const auto cachedCmyk=p.cmyk();
    require(p.edit(CM::Channel::Cyan,10),"CMYK component edit failed");near(p.cmyk().k,cachedCmyk.k);near(p.cmyk().c,10);
    p.accept();auto accepted=p.color();require(p.editHex("#FF0000"),"Hex edit failed");p.cancel();same(p.color(),accepted);
    require(p.edit(CM::Channel::LabA,127),"Lab edit failed");near(p.lab().a,127);require(p.outOfGamut(),"Lab request lost after clipping");
    require(p.edit(CM::Channel::Alpha,20),"Alpha edit failed");near(p.lab().a,127);near(p.color().a,.2);
}
void modes() {
    CM::Picker p;require(p.begin({.137,.249,.583,.37}),"Begin failed");const auto c=p.color();
    require(CM::modes().size()==9,"Missing picker modes");
    for(const auto& mode:CM::modes()) { require(p.setMode(mode.mode),"Mode rejected");same(p.color(),c);same(p.previousColor(),c);require(!mode.name.empty()&&!mode.channels.empty(),"Empty mode descriptor"); }
    require(!CM::isWebSafe(c),"Non-web-safe color reported safe");same(CM::nearestWebSafe(c),{.2,.2,.6,.37});
    require(p.edit(CM::Channel::WebRed,130),"Web edit failed");require(CM::isWebSafe(p.color()),"Web slider did not snap all components");near(p.color().r,.6);near(p.color().a,.37);
    require(p.edit(CM::Channel::Gray,100),"Gray edit failed");same(p.color(),{0,0,0,.37});require(p.edit(CM::Channel::Gray,0),"Gray edit failed");same(p.color(),{1,1,1,.37});
}
void surfaces() {
    CM::Picker black; same(black.surfaceColor(CM::Surface::HueRail,.5,1),{1,0,0,1});same(black.surfaceColor(CM::Surface::ColorWheel,1,.5),{1,0,0,1});
    CM::Picker p;require(p.begin({1,0,0,.25}),"Begin failed");same(p.surfaceColor(CM::Surface::HueCube,0,0),{1,1,1,1});same(p.surfaceColor(CM::Surface::HueCube,1,0),{1,0,0,1});same(p.surfaceColor(CM::Surface::HueCube,.5,1),{0,0,0,1});
    require(p.editSurface(CM::Surface::BrightnessCube,1/3.,0),"Brightness cube failed");same(p.color(),{0,1,0,.25});
    require(p.editSurface(CM::Surface::HueCube,.7,.3),"Hue cube failed");auto pos=p.position(CM::Surface::HueCube);near(pos.x,.7);near(pos.y,.3);
    require(p.editSurface(CM::Surface::ColorWheel,1,.5),"Wheel failed");near(p.hsb().h,0);near(p.hsb().s,100);
    require(p.surfaceColor(CM::Surface::ColorWheel,0,0).a==0,"Wheel exterior must be transparent");
    auto image=CM::renderSurface(p,CM::Surface::HueCube,3,3);require(image&&image->rgba.size()==36,"Invalid raster layout");require(image->rgba[0]==255&&image->rgba[1]==255&&image->rgba[2]==255,"Surface orientation mismatch");
    require(!CM::renderSurface(p,CM::Surface::HueCube,0,10)&&!CM::renderSurface(p,CM::Surface::HueCube,100000,100000),"Invalid dimensions accepted");
    require(!CM::renderSurface(p,CM::Surface::ChannelRamp,10,10,static_cast<CM::Channel>(99)),"Invalid channel raster accepted");
    for(auto surface:{CM::Surface::HueCube,CM::Surface::BrightnessCube,CM::Surface::ColorWheel,
                      CM::Surface::HueRail,CM::Surface::BrightnessRail,CM::Surface::Spectrum,CM::Surface::ChannelRamp})
        for(auto width:{1U,2U,17U}) for(auto height:{1U,3U,19U}) {
            const auto raster=CM::renderSurface(p,surface,width,height,CM::Channel::Alpha);
            require(bool(raster),"Reference surface rejected");
            for(unsigned y=0;y<height;++y) for(unsigned x=0;x<width;++x) {
                const auto color=p.surfaceColor(surface,width>1?double(x)/(width-1):.5,height>1?double(y)/(height-1):.5,CM::Channel::Alpha);
                const auto offset=(std::size_t(y)*width+x)*4;
                const double channels[]{color.r,color.g,color.b,color.a};
                for(unsigned channel=0;channel<4;++channel)
                    require(raster->rgba[offset+channel]==std::uint8_t(std::lround(channels[channel]*255)),"Raster differs from scalar surface sampling");
            }
        }
}
void validation() {
    CM::Picker p;const auto c=p.color();const auto nan=std::numeric_limits<double>::quiet_NaN();
    require(!p.begin({nan,0,0,1})&&!p.edit(CM::Channel::Hue,nan)&&!p.editSurface(CM::Surface::HueCube,nan,0),"NaN accepted");same(p.color(),c);
    require(!p.setMode(static_cast<CM::Mode>(99))&&!p.edit(static_cast<CM::Channel>(99),.5),"Invalid enum accepted");
    require(p.edit(CM::Channel::Red,999),"Clamped edit failed");near(p.color().r,1);require(p.edit(CM::Channel::LabB,-999),"Lab clamp failed");near(p.lab().b,-128);
}
int main(int argc,char** argv) {
    try { const std::string name=argc>1?argv[1]:"";
        if(name=="conversions")conversions();else if(name=="lab")lab();else if(name=="state")state();else if(name=="modes")modes();else if(name=="surfaces")surfaces();else if(name=="validation")validation();else return 2;
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n';return 1; }return 0;
}
