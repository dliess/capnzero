import copy
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'scripts'))
from model import build_protocol, PayloadKind
from parsing import load
from cpp.backend import render
from capnp_file import render as render_schema


class ModelTests(unittest.TestCase):
    def test_expansion_preserves_order_collisions_and_empty_payloads(self):
        raw = load(Path(__file__).with_name('fixtures') / 'Edges.toml')
        original = copy.deepcopy(raw)
        protocol = build_protocol(raw)
        self.assertEqual(raw, original)
        zebra, main = protocol.services
        self.assertEqual((zebra.ordinal, main.ordinal), (0, 1))
        self.assertEqual([m.name for m in zebra.methods], ['empty', 'noArgs', 'bytes', 'setActive', 'toggleActive'])
        self.assertEqual(zebra.methods[0].parameters.kind, PayloadKind.FIELDS)
        self.assertEqual(zebra.methods[0].parameters.fields, ())
        self.assertEqual(main.methods[0].parameters.kind, PayloadKind.VOID)
        self.assertEqual(zebra.methods[3].parameters.fields[0].name, 'val')
        self.assertEqual(zebra.methods[2].returns.type.size, 4)
        self.assertEqual(zebra.signals[0].parameters.fields[0].type.name, 'Bool')

    def test_independent_concurrent_generation(self):
        with_enum = build_protocol({'enumerations': {'Color': {'red': 0}}, 'services': {'_': {'rpc': {'f': {'parameter': {'color': 'Color'}}}}}})
        without_enum = build_protocol({'services': {'_': {'rpc': {'f': {'parameter': {'color': 'Color'}}}}}})
        models = [with_enum, without_enum] * 12
        expected = [render(p, 'Example') for p in models]
        with ThreadPoolExecutor(max_workers=4) as executor:
            actual = list(executor.map(lambda p: render(p, 'Example'), models))
        self.assertEqual(actual, expected)
        self.assertNotEqual(expected[0], expected[1])

    def test_fixed_schema_identity_is_deterministic(self):
        protocol = build_protocol({'services': {'_': {'rpc': {'f': {}}}}})
        self.assertEqual(render_schema(protocol, 'Example', '@0xdeadbeefdeadbeef'), render_schema(protocol, 'Example', '@0xdeadbeefdeadbeef'))

    def test_invalid_parameter_has_source_and_construct(self):
        with self.assertRaisesRegex(ValueError, r'broken.toml: services._.rpc.f.parameter'):
            build_protocol({'services': {'_': {'rpc': {'f': {'parameter': 'Int32'}}}}}, 'broken.toml')
