#!/usr/bin/env python3
"""Validate/repair trailing commas in a VST3 moduleinfo.json without changing plugin binaries.

JUCE 8.0.12 can emit commas before closing JSON delimiters. Run after building,
before packaging. Strict JSON validation is always performed after repair.
"""
import argparse
import json
from pathlib import Path


def strip_trailing_commas(source: str) -> str:
    out = []
    in_string = False
    escaped = False
    for index, char in enumerate(source):
        if in_string:
            out.append(char)
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                in_string = False
            continue
        if char == '"':
            in_string = True
            out.append(char)
        elif char == ",":
            following = index + 1
            while following < len(source) and source[following].isspace():
                following += 1
            if following < len(source) and source[following] in "}]":
                continue
            out.append(char)
        else:
            out.append(char)
    return "".join(out)


def repair(path: Path, fix: bool) -> bool:
    original = path.read_text(encoding="utf-8-sig")
    corrected = strip_trailing_commas(original)
    data = json.loads(corrected)
    if not isinstance(data, dict) or not isinstance(data.get("Classes"), list):
        raise ValueError("moduleinfo.json lacks a Classes array")
    changed = corrected != original
    if changed and fix:
        path.write_text(corrected, encoding="utf-8", newline="")
    elif changed:
        raise ValueError("moduleinfo.json contains trailing commas; run with --fix")
    return changed


def selftest():
    for raw, expected in [
        ('{"Classes":[],}', '{"Classes":[]}'),
        ('{"Classes":[{"Name":"x",},],}', '{"Classes":[{"Name":"x"}]}'),
        ('{"Classes":[{"Name":"comma,} and escaped \\" quote",}],}', '{"Classes":[{"Name":"comma,} and escaped \\" quote"}]}'),
        ('{"Classes":[]}', '{"Classes":[]}'),
    ]:
        assert strip_trailing_commas(raw) == expected
        json.loads(expected)
    print("Four metadata normalization regression cases passed")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("moduleinfo", nargs="?", type=Path)
    parser.add_argument("--fix", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.selftest:
        selftest()
    else:
        if args.moduleinfo is None:
            parser.error("moduleinfo path required")
        changed = repair(args.moduleinfo, args.fix)
        print("Trailing commas repaired" if changed and args.fix else "Strict JSON verified")
