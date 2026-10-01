# Generator architecture and compatibility

The compiler now follows this pipeline:

```
TOML → parsing.load → model.build_protocol → Protocol
                                              ├─ capnp_file.render
                                              ├─ cpp.backend.render
                                              └─ python_backend.render
```

`normalization.py` lowers property declarations before constructing the immutable
model. `model.py` holds ordered services, methods, signals, properties, enums,
fields, and payload shapes. Services, RPCs, and fields retain their original
ordinals. Native messages and inline Cap'n Proto text remain opaque protocol
extensions. This does not introduce optional values or new collection syntax.

`wire.py` owns wire topic strings and schema naming. C++ namespaces, signatures,
Qt mappings, and textual rendering live in `cpp/`. The C++ backend lowers the
model into a private compatibility view (`cpp/view.py`) so its established text
emitters remain recognizable. That view is not a second parser or a shared IR;
the Python backend uses `Protocol` directly. Future cleanup can replace the
private view one emitter at a time without changing either frontend or Python.
Enum definitions are explicit renderer arguments rather than mutable globals.

The generated `.capnp` schema is shared in structure across languages. The C++
target retains its existing C++ namespace annotations; the Python target omits
those annotations. Generate each target into its own output directory.

## Compatibility evidence

`tests/generator/golden/` was captured from the original generator, with only
`capnp id` controlled and clang-format disabled. It covers all seven original
IDLs and two focused edge fixtures. It includes every emitted C++, Qt,
WebChannel, interface-header, and schema file. Golden comparisons are byte for
byte. Do not regenerate expectations from the implementation under test.

The edge fixtures preserve absent versus empty payloads, declaration order,
property collisions and insertion order, and source spellings such as
`Data<04>`. Existing property collisions continue to overwrite the earlier
method/signal definition; stricter collision validation is a separate behavior
change. Unknown named types continue to pass through for inline schema use.
Structural validation adds source filename and construct paths to errors.
Exact TOML line locations are available for syntax errors from the TOML parser,
but not yet for semantic errors.

The old command line requests a new random schema identity on every invocation.
That default is retained for compatibility. For reproducible generation, supply
an explicit identity, using the same one whenever regenerating that schema:

```
python3 scripts/capnzeroc.py --descrfile=Example.toml --outdir=generated \
  --schema-id=@0xdeadbeefdeadbeef
```

Choose a separate real identity for each schema (for example with `capnp id`);
the identity above is only an example. With an explicit identity and formatting
disabled, output is deterministic. Formatter output depends on the installed
clang-format version. Formatting now uses checked subprocess arguments so paths
containing spaces work and formatter failures are reported. Short CLI options
`-d`, `-o`, and `-c` also work.

## Tests and local build

The original repository had example applications but no registered CTest tests.
On GCC 14, the pinned libzmq version fails to compile its optional CURVE code.
Use a local build option to bypass that upstream failure; the dependency pins
and default security features have not been changed. Three example utilities
also needed an explicit `<algorithm>` include.

```
python3 -m unittest discover -s tests/generator
python3 -m venv build/python-env
build/python-env/bin/python -m pip install ./python
cmake -S . -B build -DENABLE_CURVE=OFF \
  -DFETCHCONTENT_UPDATES_DISCONNECTED=ON \
  -DCAPNZERO_RUNTIME_PYTHON="$PWD/build/python-env/bin/python"
cmake --build build -j 4
ctest --test-dir build --output-on-failure
```

The generator interpreter needs `toml` (`python3-toml` in the development
container). Python generation itself does not need the Python runtime bindings.
`FETCHCONTENT_UPDATES_DISCONNECTED` is useful with an already populated
CPM dependency cache; omit it on a first checkout if dependencies need fetching.

CTest runs generator characterization, C++ interoperability, Python runtime
boundary tests, and all three pairings involving Python. Python tests are
explicitly enabled by `CAPNZERO_RUNTIME_PYTHON`; they fail if its dependencies
are unavailable. IPC socket access is required. The integration fixture checks
RPCs, signals with subscription readiness, enums, binary/text/scalar values,
native messages, direct/structured returns, subservices, and one-way calls.
Subprocess timeouts and cleanup prevent hung servers from leaking.

For a staged CMake install (the existing install configuration uses absolute
paths), use `DESTDIR=/tmp/capnzero-stage cmake --install build`. Python runtime
packaging is independent of the C++ installation.

## Remaining boundaries

The first Python backend is synchronous. Event-loop adapters, async RPCs, and
Qt-like cached property values are not provided. The protocol has no request IDs
or error-response envelope: void calls do not acknowledge delivery, and a server
handler exception cannot be sent back as a typed remote exception. Python
clients close a timed-out RPC connection to prevent a late response from being
mistaken for a subsequent call's result. These are protocol constraints, not
new wire semantics.

GUI examples are built, but automated integration uses a headless fixture.
The shared IR intentionally leaves inline Cap'n Proto schemas opaque; modeling
all Cap'n Proto constructs is outside this refactor.
