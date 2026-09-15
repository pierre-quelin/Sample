#!/usr/bin/env python3
"""Convert a Cppcheck XML version 2 report to SARIF 2.1.0."""

import argparse
import json
import xml.etree.ElementTree as ET
from pathlib import Path
from urllib.parse import quote


LEVELS = {
    "error": "error",
    "warning": "warning",
    "style": "note",
    "performance": "note",
    "portability": "note",
    "information": "note",
}


def relative_uri(filename, source_root):
    path = Path(filename)
    root = Path(source_root).resolve() if source_root else Path.cwd().resolve()
    if not path.is_absolute():
        path = (Path.cwd() / path).resolve()
    try:
        relative = path.relative_to(root)
    except ValueError:
        return None
    return quote(relative.as_posix(), safe="/-._~")


def positive_integer(value):
    if value and value.isdigit() and int(value) > 0:
        return int(value)
    return None


def convert(input_path, output_path, source_root):
    root = ET.parse(input_path).getroot()
    results = []
    rules = {}

    for finding in root.findall("./errors/error"):
        rule_id = finding.get("id") or "cppcheck"
        severity = finding.get("severity", "warning")
        location_node = finding.find("./location")
        rules.setdefault(
            rule_id,
            {
                "id": rule_id,
                "name": rule_id,
                "shortDescription": {"text": finding.get("msg", rule_id)},
                "helpUri": "https://sourceforge.net/p/cppcheck/wiki/ListOfErrors/",
            },
        )

        location = {}
        filename = finding.get("file0") or (
            location_node.get("file") if location_node is not None else None
        )
        line = finding.get("line0") or (
            location_node.get("line") if location_node is not None else None
        )
        column = None
        if filename:
            region = {}
            uri = relative_uri(filename, source_root)
            if uri is None:
                filename = None
            else:
                start_line = positive_integer(line)
                start_column = positive_integer(column)
                if start_line is not None:
                    region["startLine"] = start_line
                if start_column is not None:
                    region["startColumn"] = start_column
                physical = {"artifactLocation": {"uri": uri}}
            if filename is None:
                location = {}
            elif region:
                physical["region"] = region
                location = {"physicalLocation": physical}
            else:
                location = {"physicalLocation": physical}

        result = {
            "ruleId": rule_id,
            "level": LEVELS.get(severity, "warning"),
            "message": {"text": finding.get("verbose", finding.get("msg", rule_id))},
        }
        if location:
            result["locations"] = [location]
        results.append(result)

    sarif = {
        "$schema": "https://json.schemastore.org/sarif-2.1.0.json",
        "version": "2.1.0",
        "runs": [
            {
                "tool": {
                    "driver": {
                        "name": "Cppcheck",
                        "informationUri": "https://cppcheck.sourceforge.io/",
                        "rules": list(rules.values()),
                    }
                },
                "results": results,
            }
        ],
    }
    with open(output_path, "w", encoding="utf-8") as output:
        json.dump(sarif, output, indent=2)
        output.write("\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", help="Cppcheck XML version 2 report")
    parser.add_argument("output", help="SARIF output path")
    parser.add_argument("--source-root", default="", help="Root used for relative source URIs")
    args = parser.parse_args()
    convert(args.input, args.output, args.source_root)


if __name__ == "__main__":
    main()