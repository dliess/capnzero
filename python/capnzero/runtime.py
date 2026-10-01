"""Synchronous CapnZero transport. Each instance belongs to one thread.

RPC uses DEALER/ROUTER, with coordinates followed by an optional payload.
Void RPCs are one-way. Signals use an exact topic frame and optional payload.
All payloads are unpacked, framed Cap'n Proto messages.
"""
from dataclasses import dataclass
from enum import IntEnum
from pathlib import Path
from typing import Callable

import capnp
import zmq


def load_schema(path):
    # A parser per generated module avoids global schema-ID collisions.
    parser = capnp.SchemaParser()
    schema = parser.load(str(path), imports=[str(Path(capnp.__file__).parent.parent)])
    return parser, schema


@dataclass(frozen=True)
class Field:
    wire_name: str
    python_name: str
    enum: type[IntEnum] | None = None
    size: int | None = None

    def encode(self, value):
        if self.size is not None and len(value) != self.size:
            raise ValueError(f'{self.wire_name}: expected {self.size} bytes, got {len(value)}')
        return int(value) if self.enum is not None else value

    def decode(self, value):
        if self.enum is not None:
            return self.enum(value.raw)
        if self.size is not None:
            self.encode(value)
        if hasattr(value, 'to_dict'):
            return value.to_dict()
        return value


@dataclass(frozen=True)
class Payload:
    kind: str
    schema: object = None
    fields: tuple[Field, ...] = ()
    result_type: type | None = None

    def encode_arguments(self, arguments):
        if self.kind == 'void':
            if arguments:
                raise ValueError('Void payload cannot have arguments')
            return None
        if self.kind == 'native':
            if len(arguments) != 1:
                raise ValueError('Native payload requires one serialized message')
            return bytes(arguments[0])
        if len(arguments) != len(self.fields):
            raise ValueError('Incorrect payload field count')
        values = {f.wire_name: f.encode(value) for f, value in zip(self.fields, arguments)}
        return self.schema.new_message(**values).to_bytes()

    def decode_arguments(self, data):
        if self.kind == 'void':
            if data is not None:
                raise ValueError('Unexpected payload for void message')
            return ()
        if data is None:
            raise ValueError('Missing payload frame')
        if self.kind == 'native':
            return (data,)
        with self.schema.from_bytes(data) as reader:
            return tuple(f.decode(getattr(reader, f.wire_name)) for f in self.fields)

    def encode_result(self, value):
        if self.kind in ('direct', 'native'):
            return self.encode_arguments((value,))
        if self.kind == 'void':
            return None
        return self.encode_arguments(tuple(getattr(value, f.python_name) for f in self.fields))

    def decode_result(self, data):
        values = self.decode_arguments(data)
        if self.kind == 'void':
            return None
        if self.kind in ('direct', 'native'):
            return values[0]
        return self.result_type(*values)


@dataclass(frozen=True)
class Method:
    service_id: int
    rpc_id: int
    parameters: Payload
    returns: Payload


class Client:
    def __init__(self, schema, rpc_address=None, signal_address=None, *, context=None, timeout_ms=5000):
        self._schema = schema
        self._context = context if context is not None else zmq.Context.instance()
        self._timeout_ms = timeout_ms
        self._rpc = None
        self._signals = None
        self._callbacks = {}
        try:
            if rpc_address is not None:
                self._rpc = self._context.socket(zmq.DEALER)
                self._rpc.setsockopt(zmq.SNDTIMEO, timeout_ms)
                self._rpc.connect(rpc_address)
            if signal_address is not None:
                self._signals = self._context.socket(zmq.SUB)
                self._signals.connect(signal_address)
        except Exception:
            self.close()
            raise

    def _call(self, method, arguments):
        if self._rpc is None:
            raise RuntimeError('RPC socket is not connected or has been closed')
        coord = self._schema.RpcCoord.new_message(serviceId=method.service_id, rpcId=method.rpc_id).to_bytes()
        payload = method.parameters.encode_arguments(arguments)
        frames = [coord] if payload is None else [coord, payload]
        self._rpc.send_multipart(frames)
        if method.returns.kind == 'void':
            return None
        if not self._rpc.poll(self._timeout_ms):
            # The protocol has no request IDs. A late reply must never become
            # the next call's result, so a timed-out connection is unusable.
            self._rpc.close(linger=0)
            self._rpc = None
            raise TimeoutError('CapnZero RPC timed out; create a new client to reconnect')
        frames = self._rpc.recv_multipart()
        if len(frames) != 1:
            raise ValueError('Expected a single RPC response frame')
        return method.returns.decode_result(frames[0])

    def _on(self, topic: bytes, payload: Payload, callback: Callable):
        if self._signals is None:
            raise RuntimeError('Signal socket is not connected')
        if topic not in self._callbacks:
            self._signals.setsockopt(zmq.SUBSCRIBE, topic)
        self._callbacks[topic] = (payload, callback)

    def poll_signal(self, timeout_ms=0):
        """Dispatch one signal on the caller's thread; return whether handled."""
        if self._signals is None:
            raise RuntimeError('Signal socket is not connected')
        if not self._signals.poll(timeout_ms):
            return False
        frames = self._signals.recv_multipart()
        if not frames:
            raise ValueError('Missing signal topic')
        entry = self._callbacks.get(frames[0])
        if entry is None:  # ZeroMQ subscriptions match prefixes; we match exactly.
            return False
        payload, callback = entry
        expected = 1 if payload.kind == 'void' else 2
        if len(frames) != expected:
            raise ValueError('Incorrect signal frame count')
        callback(*payload.decode_arguments(frames[1] if expected == 2 else None))
        return True

    def close(self):
        for socket in (self._rpc, self._signals):
            if socket is not None:
                socket.close(linger=0)
        self._rpc = self._signals = None

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.close()


class Server:
    def __init__(self, schema, handlers, rpc_address=None, signal_address=None, *, context=None):
        self._schema = schema
        self._handlers = handlers
        self._context = context if context is not None else zmq.Context.instance()
        self._rpc = None
        self._signals = None
        self._subscriptions = {}
        try:
            if rpc_address is not None:
                self._rpc = self._context.socket(zmq.ROUTER)
                self._rpc.bind(rpc_address)
            if signal_address is not None:
                self._signals = self._context.socket(zmq.XPUB)
                self._signals.setsockopt(zmq.XPUB_VERBOSE, 1)
                self._signals.bind(signal_address)
        except Exception:
            self.close()
            raise

    def process_request(self, timeout_ms=0):
        """Dispatch one RPC. Handler exceptions propagate to the serving loop."""
        if self._rpc is None:
            raise RuntimeError('RPC socket is not bound')
        if not self._rpc.poll(timeout_ms):
            return False
        frames = self._rpc.recv_multipart()
        if len(frames) < 2:
            raise ValueError('Missing RPC coordinates')
        route, coord = frames[:2]
        with self._schema.RpcCoord.from_bytes(coord) as reader:
            key = (reader.serviceId, reader.rpcId)
        if key not in self._handlers:
            raise ValueError(f'Unknown RPC coordinates: {key}')
        method, handler = self._handlers[key]
        expected = 2 if method.parameters.kind == 'void' else 3
        if len(frames) != expected:
            raise ValueError('Incorrect RPC frame count')
        args = method.parameters.decode_arguments(frames[2] if expected == 3 else None)
        result = handler(*args)
        response = method.returns.encode_result(result)
        if response is not None:
            self._rpc.send_multipart([route, response])
        return True

    def _emit(self, topic, payload, arguments):
        if self._signals is None:
            raise RuntimeError('Signal socket is not bound')
        data = payload.encode_arguments(arguments)
        self._signals.send_multipart([topic] if data is None else [topic, data])

    def on_subscribe(self, topic: str, callback: Callable):
        """Register a callback to publish initial state after subscription."""
        self._subscriptions[topic.encode()] = callback

    def process_subscription(self, timeout_ms=0):
        if self._signals is None:
            raise RuntimeError('Signal socket is not bound')
        if not self._signals.poll(timeout_ms):
            return False
        message = self._signals.recv()
        if message and message[0] == 1:
            callback = self._subscriptions.get(message[1:])
            if callback is not None:
                callback()
        return True

    def close(self):
        for socket in (self._rpc, self._signals):
            if socket is not None:
                socket.close(linger=0)
        self._rpc = self._signals = None

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.close()
