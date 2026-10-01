"""Python bindings for the common protocol model."""
import keyword
import re

from model import PayloadKind
from wire import (
    upperfirst, lowerfirst, create_signal_key_name,
    create_capnp_rpc_parameter_type_str, create_capnp_rpc_return_type_str,
    create_capnp_signal_param_type_str,
)


def identifier(name):
    result = re.sub(r'\W', '_', name)
    if not result or result[0].isdigit():
        result = '_' + result
    if keyword.iskeyword(result) or result == 'self':
        result += '_'
    return result


def snake(name):
    name = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1_\2', name)
    return identifier(re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', name).lower())


def method_name(service, name):
    value = snake(name) if service == '_' else snake(service) + '__' + snake(name)
    if value in {'close', 'poll_signal', 'process_request', 'process_subscription', 'on_subscribe'} or value.startswith('_'):
        value = 'rpc_' + value
    return value


def annotation(type_, enums):
    if type_.name in enums:
        return identifier(type_.name)
    if type_.name == 'Bool':
        return 'bool'
    if re.fullmatch(r'U?Int(8|16|32|64)', type_.name):
        return 'int'
    if type_.name in ('Float32', 'Float64'):
        return 'float'
    if type_.name == 'Text':
        return 'str'
    if type_.name in ('Data', 'Span'):
        return 'bytes'
    return 'Any'  # Opaque inline Cap'n Proto type.


def parameters(payload, enums):
    if payload.kind == PayloadKind.NATIVE:
        return [('message', 'bytes')]
    return [(identifier(f.name), annotation(f.type, enums)) for f in payload.fields]


def tuple_expr(values):
    return '(' + ', '.join(values) + (',' if values else '') + ')'


def field_expr(field, enums):
    enum = identifier(field.type.name) if field.type.name in enums else 'None'
    return f'_Field({field.name!r}, {identifier(field.name)!r}, {enum}, {field.type.size!r})'


def payload_expr(payload, schema_name, enums, result='None'):
    kind = payload.kind.value
    if payload.kind in (PayloadKind.VOID, PayloadKind.NATIVE):
        return f'_Payload({kind!r})'
    if payload.kind == PayloadKind.DIRECT:
        from model import Field
        fields = (Field('retParam', payload.type, 0),)
    else:
        fields = payload.fields
    return f'_Payload({kind!r}, getattr(_schema, {schema_name!r}), {tuple_expr([field_expr(f, enums) for f in fields])}, {result})'


def render(protocol, name):
    enums = {enum.name for enum in protocol.enumerations}
    lines = [
        '"""Generated CapnZero bindings. Do not edit."""',
        'from __future__ import annotations',
        'from dataclasses import dataclass',
        'from enum import IntEnum',
        'from pathlib import Path',
        'from typing import Any, Callable, Protocol',
        'from capnzero.runtime import (Client as _Client, Server as _Server,',
        '    Field as _Field, Payload as _Payload, Method as _Method, load_schema as _load_schema)',
        '', f'_parser, _schema = _load_schema(Path(__file__).with_name({(name + ".capnp")!r}))',
        'schema = _schema', '',
    ]
    symbols = {'Client', 'Server', 'Handler', 'Path', 'Any', 'Callable', 'Protocol',
               'IntEnum', 'dataclass', 'schema', 'int', 'float', 'bool', 'str', 'bytes'}
    def reserve(value):
        if value in symbols or value.startswith('_'):
            raise ValueError(f'Python symbol collision: {value}')
        symbols.add(value)
    for enum in protocol.enumerations:
        cls = identifier(enum.name)
        reserve(cls)
        lines.append(f'class {cls}(IntEnum):')
        members = set()
        for key, value in enum.members:
            member = identifier(key)
            if member in members or member.startswith('_'):
                raise ValueError(f'Python enum member collision: {enum.name}.{key}')
            members.add(member)
            lines.append(f'    {member} = {value}')
        if not enum.members:
            lines.append('    pass')
        lines.append('')
    methods, signals = [], []
    public_methods, public_signals = set(), set()
    for service in protocol.services:
        for method in service.methods:
            pyname = method_name(service.name, method.name)
            if pyname in public_methods:
                raise ValueError(f'Python method collision: {pyname}')
            public_methods.add(pyname)
            result = 'None'
            returns = method.returns
            if returns.kind == PayloadKind.FIELDS:
                result = identifier(('' if service.name == '_' else upperfirst(service.name)) + upperfirst(method.name) + 'Result')
                reserve(result)
                lines += ['@dataclass(frozen=True)', f'class {result}:']
                field_names = [identifier(f.name) for f in returns.fields]
                if len(set(field_names)) != len(field_names):
                    raise ValueError(f'Python return field collision: {pyname}')
                lines += [f'    {identifier(f.name)}: {annotation(f.type, enums)}' for f in returns.fields] or ['    pass']
                lines.append('')
            elif returns.kind == PayloadKind.DIRECT:
                result = annotation(returns.type, enums)
            elif returns.kind == PayloadKind.NATIVE:
                result = 'bytes'
            spec = f'_rpc_{service.ordinal}_{method.ordinal}'
            param = payload_expr(method.parameters, create_capnp_rpc_parameter_type_str(service.name, method.name), enums)
            ret = payload_expr(returns, create_capnp_rpc_return_type_str(service.name, method.name), enums,
                               result if returns.kind == PayloadKind.FIELDS else 'None')
            lines += [f'{spec} = _Method({service.ordinal}, {method.ordinal}, {param}, {ret})', '']
            args = parameters(method.parameters, enums)
            if len({a for a, _ in args}) != len(args):
                raise ValueError(f'Python parameter collision: {pyname}')
            methods.append((pyname, result, spec, args, service.ordinal, method.ordinal))
        for index, signal in enumerate(service.signals):
            pyname = method_name(service.name, signal.name)
            if pyname in public_signals:
                raise ValueError(f'Python signal collision: {pyname}')
            public_signals.add(pyname)
            spec = f'_signal_{service.ordinal}_{index}'
            value = payload_expr(signal.parameters, create_capnp_signal_param_type_str(service.name, signal.name), enums)
            lines += [f'{spec} = {value}', '']
            args = parameters(signal.parameters, enums)
            if len({a for a, _ in args}) != len(args):
                raise ValueError(f'Python signal parameter collision: {pyname}')
            signals.append((pyname, spec, args, create_signal_key_name(service.name, signal.name).encode()))
    if public_methods & {'on_' + name for name in public_signals}:
        raise ValueError('Python RPC name collides with signal subscription method')
    lines += ['class Handler(Protocol):', '    """Implement RPC methods; void methods are one-way."""']
    for pyname, result, spec, args, _, _ in methods:
        signature = ''.join(f', {arg}: {ann}' for arg, ann in args)
        lines += [f'    def {pyname}(self{signature}) -> {result}: ...']
    lines += ['', 'class Client(_Client):',
              '    def __init__(self, rpc_address=None, signal_address=None, *, context=None, timeout_ms=5000):',
              '        super().__init__(_schema, rpc_address, signal_address, context=context, timeout_ms=timeout_ms)', '']
    for pyname, result, spec, args, _, _ in methods:
        signature = ''.join(f', {arg}: {ann}' for arg, ann in args)
        lines += [f'    def {pyname}(self{signature}) -> {result}:',
                  f'        return self._call({spec}, {tuple_expr([a for a, _ in args])})', '']
    for pyname, spec, args, topic in signals:
        callback = 'Callable[[' + ', '.join(ann for _, ann in args) + '], None]'
        lines += [f'    def on_{pyname}(self, callback: {callback}) -> None:',
                  f'        self._on({topic!r}, {spec}, callback)', '']
    lines += ['class Server(_Server):',
              '    def __init__(self, handler: Handler, rpc_address=None, signal_address=None, *, context=None):',
              '        handlers = {']
    for pyname, _, spec, _, sid, rid in methods:
        lines.append(f'            ({sid}, {rid}): ({spec}, handler.{pyname}),')
    lines += ['        }', '        super().__init__(_schema, handlers, rpc_address, signal_address, context=context)', '']
    for pyname, spec, args, topic in signals:
        signature = ''.join(f', {arg}: {ann}' for arg, ann in args)
        lines += [f'    def emit_{pyname}(self{signature}) -> None:',
                  f'        self._emit({topic!r}, {spec}, {tuple_expr([a for a, _ in args])})', '']
    return {name + '_capnzero.py': '\n'.join(lines)}
