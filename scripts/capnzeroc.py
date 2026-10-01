#!/usr/bin/python3
"""CapnZero compiler entry point; importing this module performs no I/O."""
import getopt
from pathlib import Path
import re
import subprocess
import sys

from parsing import load
from model import build_protocol
from capnp_file import render as render_schema
from cpp.backend import render as render_cpp


def generate(descrfile, outdir, clang_format='OFF', *, language='cpp', schema_id=None,
             capnp_executable='capnp'):
    if language not in ('cpp', 'python'):
        raise ValueError(f'Unknown target language: {language}')
    if schema_id is not None and not re.fullmatch(r'@0x[89a-fA-F][0-9a-fA-F]{15}', schema_id):
        raise ValueError('Schema ID must be @0x followed by 16 hex digits with the high bit set')
    protocol = build_protocol(load(descrfile), str(descrfile))
    name = Path(descrfile).stem
    files = {name + '.capnp': render_schema(protocol, name, schema_id,
                                          cpp_namespace=language == 'cpp',
                                          capnp_executable=capnp_executable)}
    if language == 'cpp':
        files.update(render_cpp(protocol, name))
    elif language == 'python':
        from python_backend import render
        files.update(render(protocol, name))
    output = Path(outdir)
    output.mkdir(parents=True, exist_ok=True)
    for filename, content in files.items():
        (output / filename).write_text(content)
    if clang_format in ('ON', 'on', 'On'):
        for filename in files:
            if filename.endswith(('.h', '.inl', '.cpp')):
                subprocess.run(['clang-format', '-style=file', '-i', str(output / filename)], check=True)
    return tuple(output / filename for filename in files)


def main(argv=None):
    options, remainder = getopt.getopt(sys.argv[1:] if argv is None else argv, 'o:d:c:',
                                      ['outdir=', 'descrfile=', 'clang_format=', 'language=',
                                       'schema-id=', 'capnp-executable='])
    outdir, descrfile, clang_format, language, schema_id = 'undefined', 'undefined', 'OFF', 'cpp', None
    capnp_executable = 'capnp'
    for opt, arg in options:
        if opt in ('-o', '--outdir'):
            outdir = arg
        elif opt in ('-d', '--descrfile'):
            descrfile = arg
        elif opt in ('-c', '--clang_format'):
            clang_format = arg
        elif opt == '--language':
            language = arg
        elif opt == '--schema-id':
            schema_id = arg
        elif opt == '--capnp-executable':
            capnp_executable = arg
    print('outdir: ' + outdir)
    print('descrfile: ' + descrfile)
    print('file_we: ' + Path(descrfile).stem)
    print('clang_format: ' + clang_format)
    generate(descrfile, outdir, clang_format, language=language, schema_id=schema_id,
             capnp_executable=capnp_executable)


if __name__ == '__main__':
    main()
