# Changelog

## 1.0.0 - Unreleased

- Replaced the chip-specific, global-state implementation with a reentrant C++20 model.
- Defined bit-exact two-stage interpolation, rounding, endpoint, bypass, and saturation semantics.
- Added validated P3/P6 PPM and signed LUT adapters.
- Added command-line and C interfaces.
- Added deterministic synthetic example generation.
- Added exhaustive oracle comparison, boundary, IO, and C-interface tests.
- Added CMake installation support and cross-platform continuous integration.
