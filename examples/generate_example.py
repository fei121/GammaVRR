#!/usr/bin/env python3
"""Generate copyright-safe, deterministic GammaVRR example inputs."""

from __future__ import annotations

import argparse
from pathlib import Path


def make_lut(channel_scale: float) -> list[int]:
    values: list[int] = []
    for level in range(8):
        level_strength = 7 - level
        for node in range(256):
            near_black_weight = (1.0 - node / 255.0) ** 2
            values.append(round(channel_scale * level_strength * 4 * near_black_weight))
    return values


def write_lut(path: Path, values: list[int]) -> None:
    with path.open("w", encoding="utf-8", newline="\n") as output:
        output.write("# Synthetic GammaVRR offset LUT: 8 levels x 256 nodes\n")
        for level in range(8):
            row = values[level * 256 : (level + 1) * 256]
            output.write(" ".join(str(value) for value in row))
            output.write("\n")


def write_ppm(path: Path) -> None:
    width, height = 32, 8
    with path.open("w", encoding="ascii", newline="\n") as output:
        output.write(f"P3\n# Synthetic 12-bit RGB gradient\n{width} {height}\n4095\n")
        for y in range(height):
            for x in range(width):
                base = round(x * 4095 / (width - 1))
                red = base
                green = min(4095, base + y * 8)
                blue = max(0, base - y * 8)
                output.write(f"{red} {green} {blue}\n")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output_dir", type=Path, nargs="?", default=Path("example-data"))
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    write_lut(args.output_dir / "lut_r.txt", make_lut(1.00))
    write_lut(args.output_dir / "lut_g.txt", make_lut(0.85))
    write_lut(args.output_dir / "lut_b.txt", make_lut(1.15))
    write_ppm(args.output_dir / "input.ppm")
    print(f"generated example data in {args.output_dir}")


if __name__ == "__main__":
    main()
