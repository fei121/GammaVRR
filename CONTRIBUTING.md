# Contributing

RefreshTone intentionally keeps a narrow scope. Changes should improve correctness, portability, diagnostics, or verification of the fixed 12-bit `8 x 256` model.

## Before opening a pull request

1. Preserve the behavior documented in `docs/algorithm.md`, or update the specification and explain why the compatibility break is necessary.
2. Add a focused test for every behavior change or bug fix.
3. Run the full test suite, including the exhaustive oracle comparison.
4. Run a sanitizer build on Linux or macOS when changing memory or buffer handling.
5. Do not add panel measurements, LUTs, images, or code whose redistribution rights are unclear.

General calibration pipelines, GUIs, live display control, and unrelated image effects are outside this repository's scope.
