# Photoshop reference evidence

Reference read: 2026-10-15, from the existing local project
`Engineering/Projects/Adobe/Photoshop`, recorded as Photoshop 2026 27.10.0.
No recovered Adobe implementation is copied into this SDK.

The provided screenshot establishes the nine mode names and their order.
The following recovered native interface declarations establish the observed
interaction structure:

| Local reference | Observed contract |
| --- | --- |
| `recovered/interface/Required/drover_layouts/color_panel.eve` | Foreground/background swatches, optional Gray/RGB/HSB/CMYK/Lab rows, a saturation/brightness color square, and a hue core slider |
| `recovered/interface/Required/layouts/Color/Panels/color-1590.exv` | Cube cross-section/core, gray/RGB/HSB/CMYK/Lab slider groups, hexadecimal input, `TColorWheel`, web-safe alerts and nearest-web swatch |
| `recovered/interface/Required/drover_layouts/dialogs/color_picker_dialog.eve` | New/current swatches, recent colors, numeric HSB/RGB/Lab/CMYK, accept and cancel |

The UI files name native host controls and parameter identifiers. They do not
recover the native color-transform engine or the active ICC profiles. The SDK
therefore implements independent native C++23 conversion and interaction models,
tested against reference colors and state/geometry invariants. It does not claim
Photoshop renderer, ICC, HDR, spot-color, or round-trip document parity.

Supporting primary references:

- [Adobe: Color panel](https://helpx.adobe.com/photoshop/desktop/adjust-color/choose-colors/choose-colors-in-the-color-panel.html)
- [W3C: CSS Color 4 conversions](https://www.w3.org/TR/css-color-4/#color-conversion-code)

The Adobe reference confirms numeric slider models and the 216-color web-safe
cube. W3C defines the independent sRGB/D50 Lab conversion used here. Native UI
evidence, mathematical conversion tests and Dreamscapes GUI tests are distinct
forms of evidence; none implies untested Photoshop behavior.
