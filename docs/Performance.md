# Surface rendering cost

Hue and brightness rails depend on the row coordinate only. The renderer samples
each row once and copies that exact RGBA8 pixel across the row. Horizontal ramps
and spectrum retain their existing first-row reuse. Color conversion work for a
vertical rail is O(height), while writing the output remains O(width * height).

The surface regression compares every output channel to independent scalar
`Picker::surfaceColor` sampling for all seven surface types, including one-pixel
dimensions, non-square images, wheel transparency and the alpha channel. Color
rounding, orientation, geometry limits and picker state are unchanged.

The SDK-wide performance record is under `SDK/build/performance-20261020/`.
