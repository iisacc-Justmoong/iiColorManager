# iiColorManager

The header-only `iiColorManager::Values` target provides shared encoded-sRGB
RGBA values, validation, hexadecimal parsing, and source-over composition without
linking the picker library. See [Color values](docs/ColorValues.md).

[Windows build and execution](docs/WindowsBuildAndRun.md)

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

## License

SPDX-License-Identifier: MIT

The original code and documentation in this repository are licensed under the
[MIT License](LICENSE). Third-party code, libraries, fonts, tools, and model
weights retain their respective licenses.

## Source header layout

Public headers and implementations live together in `src/iiColorManager/`.
There is no separate authored `include/` tree. Existing public `#include` names
and exported CMake targets are unchanged; installation still places only public
headers in the package include directory. Generated export headers belong to
`build/`, not to the authored source tree.

Validate this layout from the SDK workspace with
`python3 -B tests/test_source_header_layout.py`. Reconfigure, build, run CTest,
and verify a staged installed consumer after changing source/header paths.

<!-- workspace-docs:readme:start -->
## Documentation entry points

- Human overview: this English `README.md`.
- Agent instructions: [AGENT.md](AGENT.md).
- Human documentation: [documentation index](docs/README.md).
- Structured agent documentation: [agent index](docs/agent-index.json).
<!-- workspace-docs:readme:end -->
