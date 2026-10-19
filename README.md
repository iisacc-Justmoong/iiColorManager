# iiColorManager

[Windows 빌드 및 실행](docs/WindowsBuildAndRun.md)

Package release: **0.1.1** (2026-10-19). This patch packages the current
source snapshot; build, test and installation evidence is recorded in the SDK
patch-release report. Earlier versioned feature notes remain historical.


Native C++23 color conversion and color-picker state for iiSDK applications.
The exported `iiColorManager::iiColorManager` CMake target has no Qt, platform UI,
Adobe runtime, interpreter, or other SDK dependency.

The nine presentation modes are **Hue Cube**, **Brightness Cube**, **Color Wheel**,
**Grayscale Slider**, **RGB Sliders**, **HSB Sliders**, **CMYK Sliders**, **Lab
Sliders**, and **Web Color Sliders**. Mode changes preserve the exact selected
color, alpha, and cancel snapshot. The same state supplies numeric channels,
dynamic channel ramps, normalized pointer/keyboard coordinates and RGBA8 surface
images. UI adapters own event delivery, focus, translation, and drawing.

```cpp
#include <iiColorManager/ColorManager.h>
iiColorManager::Picker picker;
picker.begin({0.5, 0.25, 1.0, 0.75});
picker.setMode(iiColorManager::Mode::LabSliders);
picker.edit(iiColorManager::Channel::Lightness, 60);
auto preview = picker.color();
picker.cancel(); // Restores color and the original HSB/CMYK/Lab channel state.
```

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 3
ctest --test-dir build --output-on-failure
cmake --install build --prefix /path/to/iiColorManager
```

Consumers use `find_package(iiColorManager 0.1.1 CONFIG REQUIRED)` and
`target_link_libraries(app PRIVATE iiColorManager::iiColorManager)`.
The project builds native code using the consumer's platform toolchain; host
tests do not establish execution on other operating systems.

See [color and interaction contracts](docs/ColorModes.md) and
[Photoshop reference evidence](docs/PhotoshopEvidence.md). CMYK is a documented
generic device model, not an ICC press conversion. This package does not claim
native Photoshop pixel equivalence or arbitrary document/profile conversion.
