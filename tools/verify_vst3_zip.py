#!/usr/bin/env python3
"""Fail-closed static audit of a Windows VST3 release ZIP, without extracting it."""
import argparse
import hashlib
from pathlib import Path
import sys
import zipfile


def verify(path: Path, bundle: str) -> tuple[str, int]:
    if not path.is_file():
        raise ValueError(f"ZIP not found: {path}")
    if not bundle.endswith(".vst3") or "/" in bundle or "\\" in bundle:
        raise ValueError("bundle must be a single .vst3 directory name")
    with zipfile.ZipFile(path) as zf:
        entries = zf.infolist()
        if not entries:
            raise ValueError("empty ZIP")
        names = [entry.filename for entry in entries]
        if len(names) != len(set(name.casefold() for name in names)):
            raise ValueError("duplicate ZIP paths")
        for entry in entries:
            name = entry.filename
            parts = name.split("/")
            if (name.startswith("/") or "\\" in name or ":" in parts[0]
                    or any(part in ("", ".", "..") for part in parts)
                    or any(part.endswith((" ", ".")) for part in parts)):
                raise ValueError(f"unsafe ZIP path: {name}")
            if parts[0] != bundle:
                raise ValueError(f"unexpected top-level entry: {name}")
            if (entry.external_attr >> 16) & 0o170000 == 0o120000:
                raise ValueError(f"symlink not allowed: {name}")
        expected = f"{bundle}/Contents/x86_64-win/{bundle[:-5]}.vst3"
        if expected not in names or zf.getinfo(expected).is_dir():
            raise ValueError(f"Windows x64 binary missing: {expected}")
        if zf.getinfo(expected).file_size < 1024:
            raise ValueError("Windows VST3 binary is suspiciously small")
        with zf.open(expected) as stream:
            if stream.read(2) != b"MZ":
                raise ValueError("Windows binary does not have MZ signature")
        if zf.testzip() is not None:
            raise ValueError("ZIP CRC failure")
    with path.open("rb") as stream:
        digest = hashlib.file_digest(stream, "sha256").hexdigest()
    return digest, len(entries)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("zip", type=Path)
    parser.add_argument("--bundle", default="Skelly DeEsser.vst3")
    args = parser.parse_args()
    try:
        digest, count = verify(args.zip, args.bundle)
    except (ValueError, OSError, zipfile.BadZipFile) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1
    print(f"PASS: {args.zip} ({count} entries), SHA256 {digest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
