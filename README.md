# GammaVRR

**A bit-exact C++ Golden Model for refresh-level-dependent Gamma compensation.**

[![CI](https://img.shields.io/github/actions/workflow/status/fei121/Cmodel/ci.yml?branch=main&label=CI)](https://github.com/fei121/Cmodel/actions/workflows/ci.yml)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C.svg)](https://en.cppreference.com/w/cpp/20)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Status](https://img.shields.io/badge/status-v1.0.0%20candidate-2ea44f.svg)](CHANGELOG.md)

[Quick start](#quick-start) · [Algorithm](#how-it-works) · [Interfaces](#library-integration) · [Verification](#verification) · [Scope](#scope-and-non-goals)

GammaVRR applies refresh-level-dependent signed offset LUTs to 12-bit RGB images. Given an input image, three `8 × 256` LUTs, a `FrameLevel`, and a zero setting, it produces deterministic expected pixels for comparison with software, firmware, RTL, or silicon output.

The project focuses on a problem that is easy to underestimate in hardware-oriented image pipelines: two implementations can use the same formula and still disagree because of intermediate rounding, signed arithmetic, endpoint handling, saturation order, or buffer layout. GammaVRR makes those details explicit, executable, and exhaustively tested.

## Highlights

- **Bit-exact integer core** — no floating-point operations in the production model.
- **Executable specification** — indexing, rounding, clipping, bypass, and failure behavior are documented precisely.
- **Small, reentrant interface** — no global state; repeated and in-place processing are supported.
- **C++ and C integration** — use the native C++20 interface or a stable C-compatible seam.
- **Reproducible CLI workflow** — process P3 or standards-compliant 8/16-bit P6 PPM files.
- **Independent verification** — all `4096 × 128 × 3 = 1,572,864` sample/level/channel combinations are checked against a separately written floating-point oracle.
- **Cross-platform project** — CMake build, installable package, and CI definitions for Linux, macOS, and Windows.
- **Clean example provenance** — all example images and LUTs are generated locally from deterministic synthetic data.

## Model contract

| Property | Definition |
| --- | --- |
| Image layout | Interleaved `R, G, B` |
| Input/output | Unsigned 12-bit samples, `0…4095` |
| LUT layout | Three signed `int16` tables in level-major order |
| LUT shape | `8` refresh-level anchors × `256` gray nodes per channel |
| FrameLevel | Fixed-point interpolation coordinate, `0…127` |
| Fractions | 4-bit gray fraction and 4-bit level fraction |
| Rounding | Nearest integer; exact halves round away from zero |
| Saturation | Final output clips to `0…4095` |
| Bypass | Validated input is copied unchanged |

> [!IMPORTANT]
> `FrameLevel` is a fixed-point interpolation coordinate, not a refresh rate in hertz. Mapping a physical refresh rate to `FrameLevel` belongs to the system supplying the LUT and is intentionally outside this model.

## Quick start

### 1. Build and test

Requirements: CMake 3.20+, Git, and a C++20 compiler.

```bash
git clone https://github.com/fei121/Cmodel.git
cd Cmodel

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

CMake fetches pinned releases of [CLI11](https://github.com/CLIUtils/CLI11) and [Catch2](https://github.com/catchorg/Catch2) when the CLI and tests are enabled.

### 2. Generate copyright-safe example data

The generator uses only the Python standard library:

```bash
python3 examples/generate_example.py example-data
```

It creates a 12-bit RGB gradient and distinct synthetic R/G/B offset LUTs.

### 3. Run the model

```bash
./build/gammavrr \
  --input example-data/input.ppm \
  --output example-data/output.ppm \
  --lut-r example-data/lut_r.txt \
  --lut-g example-data/lut_g.txt \
  --lut-b example-data/lut_b.txt \
  --frame-level 73 \
  --output-format p6
```

Useful commands:

```bash
./build/gammavrr --help
./build/gammavrr --version
```

On multi-configuration generators such as Visual Studio, the executable may be under `build/Release/`.

## How it works

Each 12-bit sample and the supplied `FrameLevel` are split into an index and a 4-bit interpolation fraction:

```text
12-bit sample
├── high 8 bits ──> gray node 0…255
└── low  4 bits ──> gray interpolation fraction

FrameLevel
├── high bits ────> LUT level 0…7
└── low 4 bits ──> level interpolation fraction
```

For each R/G/B sample, the model performs:

```text
1. Interpolate two adjacent gray-node offsets at level N
2. Interpolate two adjacent gray-node offsets at level N + 1
3. Interpolate those two intermediate results across FrameLevel
4. Subtract zero_setting
5. Add the signed offset to the input sample
6. Saturate the result to 0…4095
```

In compact form:

```text
output = clamp(input + level_interp(gray_interp(LUT)) - zero_setting, 0, 4095)
```

Rounding occurs after each interpolation stage. Combining both stages into one expression is not equivalent and is not conforming. See [the complete algorithm specification](docs/algorithm.md) for the normative behavior.

## Library integration

### C++20

```cpp
#include <gammavrr/model.hpp>

gammavrr::Lut lut = load_your_lut();
gammavrr::Model model(
    {.enabled = true, .zero_setting = 0},
    std::move(lut));

const auto error = model.process(
    {input_samples, width, height},
    frame_level,
    {output_samples, width, height});

if (error != gammavrr::ProcessError::none) {
    // gammavrr::to_string(error) describes the failure.
}
```

The complete input is validated before the output is modified. Input and output may reference the same buffer.

### C interface

The C-compatible interface is declared in [`include/gammavrr/c_api.h`](include/gammavrr/c_api.h):

```c
struct gammavrr_params params = {
    .enabled = 1,
    .zero_setting = 0,
    .frame_level = 73,
};

enum gammavrr_status status = gammavrr_process(
    input, input_count,
    output, output_count,
    width, height,
    lut_r, lut_g, lut_b,
    &params);
```

Each LUT pointer supplies exactly `8 × 256` signed values in level-major order. Image buffers contain `width × height × 3` interleaved samples.

### Install and consume with CMake

```bash
cmake --install build --prefix /your/install/prefix
```

Consumer project:

```cmake
find_package(GammaVRR 1 CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE GammaVRR::gammavrr)
```

Build only the dependency-free core library:

```bash
cmake -S . -B build-core \
  -DGAMMAVRR_BUILD_CLI=OFF \
  -DGAMMAVRR_BUILD_TESTS=OFF
cmake --build build-core --parallel
```

## Verification

The test suite is designed around numerical behavior rather than code-coverage theater.

| Area | Evidence |
| --- | --- |
| Interpolation | Fixed two-stage Golden Vector plus exhaustive oracle comparison |
| Rounding | Positive and negative exact-half cases |
| Boundaries | Gray nodes, FrameLevel anchors, endpoints, and saturation |
| Safety | Dimension, buffer, frame-level, and sample validation |
| Reentrancy | Repeated and in-place processing without global state |
| File formats | P3 and portable big-endian 8/16-bit P6 round trips |
| Integration | C header compile check, C interface tests, and installed-package consumer |
| Runtime diagnostics | AddressSanitizer and UndefinedBehaviorSanitizer configuration |

Run the sanitizer configuration:

```bash
cmake -S . -B build-asan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DGAMMAVRR_ENABLE_SANITIZERS=ON
cmake --build build-asan --parallel
ctest --test-dir build-asan --output-on-failure
```

The main numerical regression test is in [`tests/model_tests.cpp`](tests/model_tests.cpp).

## Architecture

GammaVRR keeps the numerical model behind a small interface and treats files, the CLI, and the C ABI as adapters:

```text
PPM/LUT files ──> IO adapter ─┐
                              │
C++ caller ───────────────────┼──> bit-exact Model ──> output samples
                              │
C caller ───────> C adapter ──┘
```

```text
include/gammavrr/  Public C++ and C interfaces
src/               Bit-exact core and file adapters
app/               Command-line adapter
tests/             Oracle, Golden Vector, IO, and C-interface tests
docs/              Normative algorithm and release documentation
examples/          Deterministic synthetic-data generator
```

## Scope and non-goals

GammaVRR answers one question:

> Given compensation LUTs and a FrameLevel, what exact 12-bit RGB output should the digital model produce?

It intentionally does not:

- measure panel luminance or color;
- derive or optimize compensation LUTs;
- read a live refresh rate;
- process a real-time video stream;
- control a display, driver, or timing controller;
- claim to eliminate every cause of VRR flicker.

Keeping this boundary explicit makes the project useful as a trustworthy Golden Model instead of an incomplete display-calibration platform.

## Contributing

Contributions that improve correctness, portability, diagnostics, or verification of the fixed model are welcome. Read [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request.

For behavior changes, update the algorithm specification and add a focused regression test. Do not contribute code, images, LUTs, or panel measurements whose redistribution rights are unclear.

## Public-release provenance

The repository's generated examples are synthetic. Anyone publishing a version derived from employer-, customer-, or chip-owned material remains responsible for confirming that they have the right to release the implementation and algorithm.

Review [the public release checklist](docs/release-checklist.md), including Git-history provenance, before tagging a release.

## License

GammaVRR is available under the [MIT License](LICENSE).
