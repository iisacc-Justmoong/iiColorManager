# Color and interaction contracts

The canonical color is straight-alpha, encoded SDR sRGB. All RGBA components are
normalized to [0,1]. A picker rejects non-finite edits and invalid enumerators
without changing state; finite out-of-range numeric edits clamp to the channel
range. Standalone conversion helpers normalize non-finite inputs to their lower
bound. Parsing accepts CSS RGB/RGBA shorthand and full hexadecimal notation;
eight-digit strings are RRGGBBAA. Six-digit edits preserve the current alpha.

| Mode | Main interaction | Numeric channels |
| --- | --- | --- |
| Hue Cube | x = saturation, y = inverted brightness; hue rail | H 0–360°, S/B 0–100% |
| Brightness Cube | x = hue, y = inverted saturation; brightness rail | H 0–360°, S/B 0–100% |
| Color Wheel | angle = hue, radius = saturation; brightness rail | H 0–360°, S/B 0–100% |
| Grayscale Slider | neutral black ink, 0% white / 100% black | K 0–100% |
| RGB Sliders | encoded sRGB components | R/G/B 0–255 |
| HSB Sliders | hue, saturation, brightness | H 0–360°, S/B 0–100% |
| CMYK Sliders | generic black separation | C/M/Y/K 0–100% |
| Lab Sliders | D50 CIELAB, with sRGB gamut indication | L 0–100, a/b −128–127 |
| Web Color Sliders | nearest 6×6×6 web-safe cube on explicit edit | R/G/B 0–255, step 51 |

Alpha is available independently in every mode. Displaying another mode never
quantizes, converts, or otherwise edits the canonical color. A web-safe edit
rounds all three components to multiples of 51; mode selection alone does not.
HSB edits retain hue and saturation at zero brightness, and retain hue at zero
saturation. CMYK edits retain the requested separation instead of deriving a
different black component from the resulting RGB. Lab edits retain requested
out-of-gamut coordinates while the display preview clips to sRGB. `accept()`
and `cancel()` preserve/restore the full channel state, not merely RGB bytes.

The grayscale readout derives sRGB-encoded Rec.709 relative luminance; editing it
sets equal R/G/B values. CMYK uses R=(1−C)(1−K), G=(1−M)(1−K), B=(1−Y)(1−K).
It does not emulate Photoshop's active working CMYK/gray ICC profile. CIELAB
conversion uses the sRGB transfer function and matrices, Bradford D65↔D50
adaptation, and a D50 white of x=0.3457, y=0.3585. See
[W3C CSS Color 4 conversion definitions](https://www.w3.org/TR/css-color-4/#color-conversion-code).

Surface coordinates are normalized with a top-left origin. The wheel's red is
at the right, green at 120°, blue at 240°. The wheel and hue rail display full
brightness so hue remains visible for black input; editing them preserves the
selected brightness. The brightness cube intentionally displays its fixed
brightness. The wheel exterior is transparent. Other surfaces are opaque
except the alpha ramp, which preserves straight alpha for checkerboard drawing.
Raster output is contiguous row-major RGBA8, with four bytes per pixel, and a
16-million-pixel safety limit. Pointer selection and marker coordinates share
the same native model and do not require JavaScript color conversion.
