#!/usr/bin/env python3
"""Package a merged ESP32-S3 image with a web-flasher manifest."""

from __future__ import annotations

import argparse
import json
import zipfile
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--image", required=True, type=Path)
    parser.add_argument("--name", required=True)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    if not args.image.is_file():
        raise FileNotFoundError(args.image)

    image_name = f"{args.name}.bin"
    manifest = {
        "chipFamily": "ESP32-S3",
        "flashSize": "16MB",
        "flashMode": "dio",
        "flashFreq": "80m",
        "parts": [
            {"path": image_name, "offset": "0x0000"},
        ],
    }

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(args.output, "w", compression=zipfile.ZIP_DEFLATED) as zf:
        zf.write(args.image, image_name)
        zf.writestr("manifest.json", json.dumps(manifest, indent=2) + "\n")

    print(f"firmware package written to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
