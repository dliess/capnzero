# Python clients and servers

Requires Python 3.10 or newer. Install the small runtime package:

```
python -m pip install ./python
```

It uses `pycapnp` for the existing Cap'n Proto wire format and `pyzmq` for the
existing ZeroMQ transport. These bindings avoid implementing either wire stack
again. The generator itself still only needs `toml`.

Generate a module and its companion schema:

```
python3 scripts/capnzeroc.py --language=python \
  --descrfile=tests/interop/Interop.toml --outdir=generated-python
```

Keep `Interop_capnzero.py` and `Interop.capnp` together, and add that directory
to your application's import path. Use separate directories for C++ and Python
outputs. Existing C++ generation remains the default without `--language`.

```python
import Interop_capnzero as api

with api.Client("tcp://localhost:5555", "tcp://localhost:5556") as client:
    client.on_changed(lambda value: print("changed:", value))
    result = client.add(17, 25)
    assert result.value == 42
    client.poll_signal(timeout_ms=1000)
```

Methods use snake_case. Subservice methods use `service__method`, for example
`device__increment`. Python keywords are escaped with a trailing underscore.
Ambiguous generated names fail clearly. Field-table returns are frozen
dataclasses; direct returns are scalar values. Enums are `IntEnum` classes,
text is `str`, and `Data<N>`/`Span` values are `bytes`. Fixed-size data lengths
are checked during serialization and decoding. Named inline schema types use
`Any` annotations. Native payloads are serialized, unpacked Cap'n Proto bytes;
the module's public `schema` object can construct and read them.

Implement the generated `Handler` protocol and pass the implementation to
`Server`. This minimal IDL is enough for a complete application:

```toml
[services._.rpc.add]
parameter = { a = 'Int32', b = 'Int32' }
returns = { value = 'Int32' }
[services._.signal.changed]
parameter = { value = 'Int32' }
```

Save it as `Calculator.toml`, generate Python bindings, and run:

```python
import Calculator_capnzero as api

class Calculator:
    def add(self, a: int, b: int) -> api.AddResult:
        return api.AddResult(a + b)

with api.Server(Calculator(), "tcp://*:5555", "tcp://*:5556") as server:
    # Publish initial state once the subscription has actually arrived.
    server.on_subscribe("changed", lambda: server.emit_changed(42))
    while True:
        server.process_subscription(timeout_ms=10)
        server.process_request(timeout_ms=10)
```

Both client and server can be used with just the RPC or signal address for
protocols that only use one channel. Server signal methods use `emit_...`,
client subscription methods use `on_...`, and callbacks execute when the
application polls. A signals-only server can use `handler=None`. Pass a ZeroMQ
context through `context=` to control its lifetime; closing a binding closes
its sockets, not the caller's context. Instances and their sockets belong to
one thread. `close()` uses zero linger, so queued one-way sends are not a
substitute for a protocol-level delivery acknowledgment.

Property declarations generate the same setter/toggle RPCs and changed signals
as C++. The Python server handler implements those generated methods; the
application owns state and emits notifications. Read-only properties are
observed through signals, since the existing protocol does not synthesize
getter RPCs. Python does not add a local property cache.

The wire protocol has no remote exception envelope. Handler exceptions propagate
to your serving loop. Clients have a default 5000 ms RPC timeout; a timeout
closes the RPC connection, and recovery requires a new client instance.

Binding API references: [pycapnp serialization](https://capnproto.github.io/pycapnp/quickstart.html)
and [PyZMQ](https://pyzmq.readthedocs.io/).
