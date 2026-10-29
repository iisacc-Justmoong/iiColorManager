#pragma once
#include <iiColorManager/Export.h>
#include <iiColorManager/ColorValue.h>
#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace iiColorManager {
// Unassociated, encoded sRGB. Picker colors are SDR; every component is [0,1].
struct Hsb { double h=0, s=0, b=0; }; // Degrees [0,360), percent [0,100].
struct Cmyk { double c=0, m=0, y=0, k=0; }; // Generic CMYK percentages, without an ICC press profile.
struct Lab { double l=0, a=0, b=0; }; // CIELAB, D50 white; L [0,100], a/b [-128,127].
struct GamutColor { Rgba color; bool outOfGamut=false; };
struct Point { double x=0, y=0; };
enum class Mode { HueCube, BrightnessCube, ColorWheel, GrayscaleSlider, RgbSliders,
                  HsbSliders, CmykSliders, LabSliders, WebColorSliders };
// The first 12 channel values retain the existing LVRS numeric channel contract.
enum class Channel { Hue, Saturation, Brightness, Red, Green, Blue, Cyan, Magenta,
                     Yellow, Black, Alpha, Gray, Lightness, LabA, LabB, WebRed, WebGreen, WebBlue };
enum class Surface { HueCube, BrightnessCube, ColorWheel, HueRail, BrightnessRail, Spectrum, ChannelRamp };
struct ChannelInfo { Channel channel; std::string_view label, name, unit; double minimum, maximum, step; };
struct ModeInfo { Mode mode; std::string_view key, name; std::span<const Channel> channels; bool hasPlane; };
struct SurfaceImage { std::uint32_t width=0, height=0; std::vector<std::uint8_t> rgba; };

[[nodiscard]] IICOLORMANAGER_EXPORT std::span<const ModeInfo> modes() noexcept;
[[nodiscard]] IICOLORMANAGER_EXPORT const ModeInfo* modeInfo(Mode) noexcept;
[[nodiscard]] IICOLORMANAGER_EXPORT const ChannelInfo* channelInfo(Channel) noexcept;
[[nodiscard]] IICOLORMANAGER_EXPORT Hsb toHsb(Rgba) noexcept;
[[nodiscard]] IICOLORMANAGER_EXPORT Rgba fromHsb(Hsb, double alpha=1) noexcept;
[[nodiscard]] IICOLORMANAGER_EXPORT Cmyk toCmyk(Rgba) noexcept;
[[nodiscard]] IICOLORMANAGER_EXPORT Rgba fromCmyk(Cmyk, double alpha=1) noexcept;
[[nodiscard]] IICOLORMANAGER_EXPORT Lab toLab(Rgba) noexcept;
[[nodiscard]] IICOLORMANAGER_EXPORT GamutColor fromLab(Lab, double alpha=1) noexcept;
[[nodiscard]] IICOLORMANAGER_EXPORT bool isWebSafe(Rgba) noexcept;
[[nodiscard]] IICOLORMANAGER_EXPORT Rgba nearestWebSafe(Rgba) noexcept;
// CSS byte order: RGB, RGBA, RRGGBB, RRGGBBAA; an absent alpha preserves fallbackAlpha.
[[nodiscard]] IICOLORMANAGER_EXPORT std::optional<Rgba> parseHex(std::string_view, double fallbackAlpha=1) noexcept;
[[nodiscard]] IICOLORMANAGER_EXPORT std::string toHex(Rgba, bool includeAlpha=false);

class IICOLORMANAGER_EXPORT Picker final {
public:
    Picker() noexcept;
    [[nodiscard]] Mode mode() const noexcept { return mode_; }
    [[nodiscard]] bool setMode(Mode) noexcept; // Presentation only; never changes color or the cancel snapshot.
    [[nodiscard]] Rgba color() const noexcept { return state_.rgb; }
    [[nodiscard]] Rgba previousColor() const noexcept { return baseline_.rgb; }
    [[nodiscard]] Hsb hsb() const noexcept { return state_.hsb; }
    [[nodiscard]] Cmyk cmyk() const noexcept { return state_.cmyk; }
    [[nodiscard]] Lab lab() const noexcept { return state_.lab; }
    [[nodiscard]] bool outOfGamut() const noexcept { return state_.outOfGamut; }
    [[nodiscard]] double channelValue(Channel) const noexcept;
    [[nodiscard]] bool begin(Rgba) noexcept;
    [[nodiscard]] bool editColor(Rgba) noexcept;
    [[nodiscard]] bool editHex(std::string_view) noexcept;
    [[nodiscard]] bool edit(Channel, double) noexcept;
    [[nodiscard]] bool editSurface(Surface, double x, double y, Channel=Channel::Red) noexcept;
    [[nodiscard]] Rgba surfaceColor(Surface, double x, double y, Channel=Channel::Red) const noexcept;
    [[nodiscard]] Point position(Surface, Channel=Channel::Red) const noexcept;
    void accept() noexcept { baseline_=state_; }
    void cancel() noexcept { state_=baseline_; }
private:
    struct State { Rgba rgb; Hsb hsb; Cmyk cmyk; Lab lab; bool outOfGamut=false; };
    State state_, baseline_;
    Mode mode_=Mode::HueCube;
    void syncRgb() noexcept;
};
// Opaque visual surfaces, except the wheel exterior and the alpha ramp. Straight RGBA8.
// Invalid/zero dimensions and images exceeding 16 million pixels return nullopt.
[[nodiscard]] IICOLORMANAGER_EXPORT std::optional<SurfaceImage> renderSurface(
    const Picker&, Surface, std::uint32_t width, std::uint32_t height, Channel=Channel::Red);
} // namespace iiColorManager
