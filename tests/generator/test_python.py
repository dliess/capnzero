import ast
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'scripts'))
from model import build_protocol
from parsing import load
from python_backend import render
from test_golden import FIXTURES


class PythonGenerationTests(unittest.TestCase):
    def test_all_existing_examples_produce_valid_python(self):
        for schema in FIXTURES:
            with self.subTest(schema=schema.stem):
                protocol = build_protocol(load(schema))
                source = render(protocol, schema.stem)[schema.stem + '_capnzero.py']
                tree = ast.parse(source)
                compile(tree, '<generated>', 'exec')
                self.assertEqual(source, render(protocol, schema.stem)[schema.stem + '_capnzero.py'])

    def test_keyword_parameters_are_escaped_without_changing_wire_fields(self):
        protocol = build_protocol({'services': {'_': {'rpc': {'class': {
            'parameter': {'from': 'Text', 'self': 'Int32'}, 'returns': {'yield': 'Int32'}}}}}})
        source = render(protocol, 'Keywords')['Keywords_capnzero.py']
        compile(source, '<generated>', 'exec')
        self.assertIn("_Field('from', 'from_'", source)
        self.assertIn('def class_(self, from_: str, self_: int)', source)

    def test_name_collisions_fail_before_writing(self):
        protocol = build_protocol({'services': {'_': {'rpc': {'getValue': {}, 'get_value': {}}}}})
        with self.assertRaisesRegex(ValueError, 'Python method collision'):
            render(protocol, 'Collision')
