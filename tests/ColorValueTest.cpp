#include <iiColorManager/ColorValue.h>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace iiColorManager;
void check(bool value,const char* text){if(!value)throw std::runtime_error(text);}
int main(){try{
    check(validRgba({.2,.4,.6,.5})&&!validRgba({2,0,0,1})&&!validRgba({0,0,0,std::numeric_limits<double>::quiet_NaN()}),"Value validation failed");
    check(parseHexValue(" #f08 ",.25)==Rgba{1,0,136/255.,.25},"Short hex or fallback alpha changed");
    check(parseHexValue("01020304")==Rgba{1/255.,2/255.,3/255.,4/255.},"RGBA byte order changed");
    check(!parseHexValue("#12345")&&!parseHexValue("#gg0000"),"Malformed hex accepted");
    check(parseHexValue("#000",2)->a==1&&parseHexValue("#000",std::numeric_limits<double>::infinity())->a==0,"Legacy fallback alpha changed");
    check(compositeSrgb({1,0,0,.5},{0,0,1,1})==Rgba{.5,0,.5,1},"Encoded-space source-over changed");
    check(compositeSrgb({1,1,1,0},{1,1,1,0})==Rgba{0,0,0,0},"Zero alpha retained hidden channels");
    check(!compositeSrgb({2,0,0,1},{0,0,0,1}),"Invalid composition accepted");
    std::cout<<"Header-only color values passed\n";
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
