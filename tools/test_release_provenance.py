#!/usr/bin/env python3
"""Offline regression checks for the Windows VST3 provenance validator."""
import json
import tempfile
import unittest
import zipfile
from pathlib import Path

from release_provenance import inspect

ROOT = "Skelly DeEsser.vst3"
BINARY = ROOT + "/Contents/x86_64-win/Skelly DeEsser.vst3"
META = ROOT + "/Contents/Resources/moduleinfo.json"


def pe_fixture():
    data = bytearray(256)
    data[:2] = b"MZ"
    data[0x3c:0x40] = (128).to_bytes(4, "little")
    data[128:132] = bytes((80, 69, 0, 0))
    return bytes(data)


class ProvenanceTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.path = Path(self.temp.name) / "candidate.zip"

    def package(self, additions=None, metadata=None):
        entries = {
            BINARY: pe_fixture(),
            META: json.dumps({"Name": "Skelly DeEsser", "Version": "1.17.2",
                              "Classes": [{"CID": "test"}]}).encode(),
        }
        if metadata is not None:
            entries[META] = metadata
        if additions:
            entries.update(additions)
        with zipfile.ZipFile(self.path, "w", zipfile.ZIP_DEFLATED) as archive:
            for name, data in entries.items():
                archive.writestr(name, data)

    def test_valid_package(self):
        self.package()
        result = inspect(self.path)
        self.assertEqual(result["version"], "1.17.2")
        self.assertEqual(result["host_acceptance"], "UNVERIFIED")
        self.assertEqual(result["audio_acceptance"], "UNVERIFIED")

    def test_directory_entries_allowed(self):
        self.package({ROOT + "/": b""})
        self.assertEqual(inspect(self.path)["entry_count"], 3)

    def test_case_insensitive_duplicate_rejected(self):
        self.package({BINARY.upper(): b"duplicate"})
        with self.assertRaisesRegex(ValueError, "duplicate"):
            inspect(self.path)

    def test_parent_traversal_rejected(self):
        self.package({"../payload.txt": b"bad"})
        with self.assertRaisesRegex(ValueError, "unsafe"):
            inspect(self.path)

    def test_malformed_json_rejected(self):
        self.package(metadata=b'{"Classes":[{}],}')
        with self.assertRaises(ValueError):
            inspect(self.path)

    def test_missing_class_metadata_rejected(self):
        self.package(metadata=b'{"Classes":[]}')
        with self.assertRaisesRegex(ValueError, "class"):
            inspect(self.path)

    def test_invalid_pe_rejected(self):
        self.package({BINARY: b"not a PE file"})
        with self.assertRaisesRegex(ValueError, "DOS header"):
            inspect(self.path)


if __name__ == "__main__":
    unittest.main()
