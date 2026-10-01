"""Compatibility snapshots captured before restructuring the generator."""
import os
from pathlib import Path
import runpy
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
GOLDEN = Path(__file__).with_name('golden')
FIXTURES = sorted(ROOT.glob('tests/*/schema/*.toml')) + sorted(Path(__file__).with_name('fixtures').glob('*.toml'))
SCHEMA_ID = b'@0xdeadbeefdeadbeef\n'


def generate(schema, output):
    """Exercise the CLI, controlling only the external schema-ID input."""
    real_check_output = subprocess.check_output
    def check_output(args, *pos, **kwargs):
        if args == ['capnp', 'id']:
            return SCHEMA_ID
        return real_check_output(args, *pos, **kwargs)
    argv = ['capnzeroc.py', '--descrfile=' + str(schema), '--outdir=' + str(output), '--clang_format=OFF']
    with patch.object(sys, 'argv', argv), patch.object(sys, 'path', [str(ROOT / 'scripts')] + sys.path), patch('subprocess.check_output', check_output):
        runpy.run_path(str(ROOT / 'scripts/capnzeroc.py'), run_name='__main__')


class GoldenTests(unittest.TestCase):
    def test_existing_outputs(self):
        with tempfile.TemporaryDirectory() as tmp:
            for schema in FIXTURES:
                with self.subTest(schema=schema.stem):
                    out = Path(tmp) / schema.stem
                    generate(schema, out)
                    expected = GOLDEN / schema.stem
                    self.assertEqual(sorted(p.name for p in expected.iterdir()), sorted(p.name for p in out.iterdir()))
                    for path in expected.iterdir():
                        self.assertEqual(path.read_bytes(), (out / path.name).read_bytes(), path.name)

    def test_missing_services_fails(self):
        with tempfile.TemporaryDirectory() as tmp:
            schema = Path(tmp) / 'Invalid.toml'
            schema.write_text('[enumerations]\nColor = { red = 0 }\n')
            with self.assertRaises((KeyError, ValueError)):
                generate(schema, Path(tmp) / 'out')

    def test_explicit_identity_is_independent_of_paths_and_hash_seed(self):
        schema = ROOT / 'tests/Calculator/schema/Calculator.toml'
        with tempfile.TemporaryDirectory(prefix='capnzero paths ') as tmp:
            for seed in ('1', '73'):
                output = Path(tmp) / seed
                subprocess.run([
                    sys.executable, str(ROOT / 'scripts/capnzeroc.py'),
                    '-d', str(schema), '-o', str(output), '-c', 'OFF',
                    '--schema-id=@0xdeadbeefdeadbeef',
                ], env={**os.environ, 'PYTHONHASHSEED': seed},
                    check=True, capture_output=True, text=True)
                for expected in (GOLDEN / schema.stem).iterdir():
                    self.assertEqual(expected.read_bytes(), (output / expected.name).read_bytes())

    def test_invalid_identity_writes_no_files(self):
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp) / 'out'
            result = subprocess.run([
                sys.executable, str(ROOT / 'scripts/capnzeroc.py'),
                '--descrfile=' + str(FIXTURES[0]), '--outdir=' + str(output),
                '--schema-id=not-an-id',
            ], capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('Schema ID must be', result.stderr)
            self.assertFalse(output.exists())


if __name__ == '__main__':
    unittest.main()
