"""Regression tests for the release ZIP audit (stdlib only)."""
import importlib.util
from pathlib import Path
import tempfile
import unittest
import warnings
import zipfile

SCRIPT = Path(__file__).resolve().parents[1] / "tools" / "verify_vst3_zip.py"
spec = importlib.util.spec_from_file_location("verify_vst3_zip", SCRIPT)
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)

BUNDLE = "Skelly DeEsser.vst3"
BINARY = f"{BUNDLE}/Contents/x86_64-win/Skelly DeEsser.vst3"
FAKE_PE = b"MZ" + bytes(2046)


class ReleaseZipTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.zip = Path(self.tmp.name) / "candidate.zip"

    def make_zip(self, extra=(), binary=FAKE_PE, omit_binary=False):
        with zipfile.ZipFile(self.zip, "w", zipfile.ZIP_DEFLATED) as z:
            if not omit_binary:
                z.writestr(BINARY, binary)
            for name, data in extra:
                z.writestr(name, data)

    def test_valid_layout(self):
        self.make_zip()
        digest, count = audit.verify(self.zip, BUNDLE)
        self.assertEqual(len(digest), 64)
        self.assertEqual(count, 1)

    def test_missing_binary(self):
        self.make_zip(omit_binary=True, extra=[(f"{BUNDLE}/Contents/Info.plist", b"x")])
        with self.assertRaisesRegex(ValueError, "missing"):
            audit.verify(self.zip, BUNDLE)

    def test_invalid_executable(self):
        self.make_zip(binary=b"NO" + bytes(2046))
        with self.assertRaisesRegex(ValueError, "MZ"):
            audit.verify(self.zip, BUNDLE)

    def test_tiny_executable(self):
        self.make_zip(binary=b"MZ")
        with self.assertRaisesRegex(ValueError, "small"):
            audit.verify(self.zip, BUNDLE)

    def test_traversal(self):
        self.make_zip(extra=[(f"{BUNDLE}/../evil.txt", b"x")])
        with self.assertRaisesRegex(ValueError, "unsafe"):
            audit.verify(self.zip, BUNDLE)

    def test_wrong_root(self):
        self.make_zip(extra=[("other/file.txt", b"x")])
        with self.assertRaisesRegex(ValueError, "top-level"):
            audit.verify(self.zip, BUNDLE)

    def test_duplicate(self):
        with warnings.catch_warnings():
            warnings.simplefilter("ignore", UserWarning)
            self.make_zip(extra=[(BINARY, FAKE_PE)])
        with self.assertRaisesRegex(ValueError, "duplicate"):
            audit.verify(self.zip, BUNDLE)

    def test_case_insensitive_collision(self):
        self.make_zip(extra=[(BINARY.lower(), FAKE_PE)])
        with self.assertRaisesRegex(ValueError, "duplicate"):
            audit.verify(self.zip, BUNDLE)

    def test_trailing_parent_segment(self):
        self.make_zip(extra=[(f"{BUNDLE}/Contents/..", b"x")])
        with self.assertRaisesRegex(ValueError, "unsafe"):
            audit.verify(self.zip, BUNDLE)

    def test_trailing_dot_windows_path(self):
        self.make_zip(extra=[(f"{BUNDLE}/Contents/evil.", b"x")])
        with self.assertRaisesRegex(ValueError, "unsafe"):
            audit.verify(self.zip, BUNDLE)

    def test_bad_bundle_argument(self):
        self.make_zip()
        with self.assertRaisesRegex(ValueError, "bundle"):
            audit.verify(self.zip, "../bad.vst3")

    def test_corrupt_zip(self):
        self.zip.write_bytes(b"not a zip")
        with self.assertRaises(zipfile.BadZipFile):
            audit.verify(self.zip, BUNDLE)


if __name__ == "__main__":
    unittest.main()
