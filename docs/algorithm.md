# RefreshTone Algorithm Specification

## Scope

RefreshTone applies refresh-level-dependent signed offsets to one interleaved RGB image. It is a deterministic digital reference model. It does not measure a panel, generate calibration LUTs, or control display hardware.

The fixed model shape is:

- input and output: unsigned 12-bit samples, `0..4095`;
- channels: interleaved `R, G, B`;
- LUT: one signed `int16` table per channel;
- LUT shape: 8 refresh-level anchors by 256 gray nodes;
- frame level: integer `0..127`;
- interpolation fractions: 4 bits.

## Index decomposition

For an input sample `x`:

```text
gray0    = x >> 4
gray1    = min(gray0 + 1, 255)
grayFrac = x & 0x0f
```

For a frame level `f`:

```text
level0    = f >> 4
level1    = min(level0 + 1, 7)
levelFrac = f & 0x0f
```

The endpoint is clamped. Therefore sample 4095 uses node 255 for both gray endpoints, and frame level 127 uses level 7 for both level endpoints.

## Interpolation and rounding

The primitive interpolation is:

```text
interp(a, b, t) = round_away_from_zero((a * (16 - t) + b * t) / 16)
```

`t` is in `0..15`. Rounding is to the nearest integer; an exact half is rounded away from zero. This explicitly preserves the intent of the original `std::round` implementation without using floating-point arithmetic.

For channel `c`:

```text
atLevel0 = interp(lut[c][level0][gray0], lut[c][level0][gray1], grayFrac)
atLevel1 = interp(lut[c][level1][gray0], lut[c][level1][gray1], grayFrac)
offset   = interp(atLevel0, atLevel1, levelFrac) - zeroSetting
output   = clamp(x + offset, 0, 4095)
```

Rounding happens after each interpolation stage. Collapsing both stages into one expression can produce different results and is not conforming.

## Bypass and errors

- When `enabled` is false, validated input is copied unchanged to output.
- Frame levels outside `0..127` are rejected.
- Zero dimensions, overflowing dimensions, mismatched buffers, and samples above 4095 are rejected.
- The operation is all-or-nothing for validation errors: all samples are validated before output is modified.
- In-place processing is supported.

## LUT text format

Each channel file contains exactly 2048 whitespace-separated signed decimal integers in level-major order:

```text
level 0: nodes 0..255
level 1: nodes 0..255
...
level 7: nodes 0..255
```

Lines may contain comments beginning with `#` or `//`.
