#!/usr/bin/env python3
"""Read-only Windows VST3 ZIP provenance generator; rejects invalid packages."""
import argparse
import hashlib
import json
from pathlib import Path
import zipfile

def digest(data):
    return hashlib.sha256(data).hexdigest()

def inspect(path):
    with zipfile.ZipFile(path) as z:
        names = z.namelist()
        if len(names) != len({n.casefold() for n in names}):
            raise ValueError("duplicate case-insensitive paths")
        if z.testzip() is not None:
            raise ValueError("ZIP CRC failure")
        if any(n.startswith("/") or "\\" in n or ".." in n.split("/") for n in names):
            raise ValueError("unsafe ZIP member path")
        binaries = [n for n in names if n.endswith("/Contents/x86_64-win/Skelly DeEsser.vst3")]
        metas = [n for n in names if n.endswith("/Contents/Resources/moduleinfo.json")]
        if len(binaries) != 1 or len(metas) != 1:
            raise ValueError("expected exactly one Windows VST3 binary and moduleinfo.json")
        root = binaries[0].split("/Contents/")[0]
        if not root.endswith(".vst3") or metas[0].split("/Contents/")[0] != root:
            raise ValueError("bundle roots differ")
        binary, metadata = z.read(binaries[0]), z.read(metas[0])
        info = json.loads(metadata)
        if binary[:2] != b"MZ" or len(binary) < 64:
            raise ValueError("invalid Windows binary DOS header")
        offset = int.from_bytes(binary[0x3c:0x40], "little")
        if binary[offset:offset+4] != b"PE\0\0":
            raise ValueError("invalid Windows PE signature")
        if not isinstance(info.get("Classes"), list) or not info["Classes"]:
            raise ValueError("missing VST3 class metadata")
        return dict(archive=path.name, archive_sha256=digest(path.read_bytes()),
                    name=info.get("Name"), version=info.get("Version"),
                    bundle_root=root, binary_path=binaries[0],
                    binary_sha256=digest(binary), metadata_sha256=digest(metadata),
                    entry_count=len(names), host_acceptance="UNVERIFIED",
                    audio_acceptance="UNVERIFIED")

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    try:
        output = json.dumps(inspect(args.archive), sort_keys=True, indent=2) + "\n"
        if args.output:
            args.output.write_text(output, encoding="utf-8")
        else:
            print(output, end="")
    except (OSError, ValueError, zipfile.BadZipFile, KeyError, IndexError) as e:
        parser.exit(1, f"validation failed: {e}\n")

if __name__ == "__main__":
    main()
