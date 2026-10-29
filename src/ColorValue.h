#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <string_view>

namespace iiColorManager {
// Unassociated, encoded sRGB. This header-only value contract has no GUI/runtime dependency.
struct Rgba {
    static constexpr bool encoded_srgb = true;
    double r=0,g=0,b=0,a=1;
    bool operator==(const Rgba&) const = default;
};
[[nodiscard]] inline bool validRgba(Rgba c) noexcept {
    const auto channel=[](double v){return std::isfinite(v)&&v>=0&&v<=1;};
    return channel(c.r)&&channel(c.g)&&channel(c.b)&&channel(c.a);
}
// CSS byte order, optional '#'; RGB/RGBA/RRGGBB/RRGGBBAA. Preserve the picker
// parser's fallback-alpha clamping, including its nonfinite-to-zero behavior.
[[nodiscard]] inline std::optional<Rgba> parseHexValue(std::string_view text,double alpha=1) noexcept {
    const auto space=[](char c){return c==' '||c=='\t'||c=='\n'||c=='\r';};
    while(!text.empty()&&space(text.front()))text.remove_prefix(1);
    while(!text.empty()&&space(text.back()))text.remove_suffix(1);
    if(!text.empty()&&text.front()=='#')text.remove_prefix(1);
    const auto size=text.size();if(size!=3&&size!=4&&size!=6&&size!=8)return {};
    std::array<int,8> digits{};
    for(std::size_t i=0;i<size;++i){const char c=text[i];digits[i]=c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:-1;if(digits[i]<0)return {};}
    const bool short_form=size<5;
    const auto channel=[&](std::size_t i){return (short_form?digits[i]*17:digits[i*2]*16+digits[i*2+1])/255.;};
    return Rgba{channel(0),channel(1),channel(2),(size==4||size==8)?channel(3):(std::isfinite(alpha)?std::clamp(alpha,0.,1.):0.)};
}
// Encoded-space source-over, matching authored UI tokens. Linear-light image
// compositing is a different operation and must be requested by its own API.
[[nodiscard]] inline std::optional<Rgba> compositeSrgb(Rgba front,Rgba back) noexcept {
    if(!validRgba(front)||!validRgba(back))return {};
    const auto alpha=front.a+back.a*(1-front.a);
    if(alpha==0)return Rgba{0,0,0,0};
    const auto channel=[&](double a,double b){return (a*front.a+b*back.a*(1-front.a))/alpha;};
    return Rgba{channel(front.r,back.r),channel(front.g,back.g),channel(front.b,back.b),alpha};
}
}
