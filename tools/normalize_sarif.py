#!/usr/bin/env python3
"""Remove invalid zero-based SARIF column values from tool-generated reports."""

import json
import sys


def normalize(value):
    if isinstance(value, list):
        return [normalize(item) for item in value]
    if isinstance(value, dict):
        normalized = {
            key: normalize(item)
            for key, item in value.items()
            if key not in {"startColumn", "endColumn"}
            or not isinstance(item, int)
            or item >= 1
        }
        return normalized
    return value


def main():
    if len(sys.argv) != 3:
        raise SystemExit(f"usage: {sys.argv[0]} INPUT OUTPUT")
    with open(sys.argv[1], encoding="utf-8") as input_file:
        report = json.load(input_file)
    with open(sys.argv[2], "w", encoding="utf-8") as output_file:
        json.dump(normalize(report), output_file, indent=2)
        output_file.write("\n")


if __name__ == "__main__":
    main()