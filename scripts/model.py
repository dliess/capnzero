"""Ordered, language-neutral CapnZero protocol values.

Opaque Cap'n Proto types and inline schema text remain protocol extensions;
this model deliberately does not parse or reinterpret Cap'n Proto syntax.
"""
from dataclasses import dataclass
from enum import Enum
import re


class PayloadKind(Enum):
    VOID = 'void'
    FIELDS = 'fields'
    NATIVE = 'native'
    DIRECT = 'direct'


@dataclass(frozen=True)
class Type:
    name: str
    size: int | None = None
    # Preserve IDL lexemes such as Data<04> for exact legacy rendering.
    source_spelling: str | None = None


@dataclass(frozen=True)
class Field:
    name: str
    type: Type
    ordinal: int


@dataclass(frozen=True)
class Payload:
    kind: PayloadKind
    fields: tuple[Field, ...] = ()
    type: Type | None = None


@dataclass(frozen=True)
class Method:
    name: str
    ordinal: int
    parameters: Payload
    returns: Payload


@dataclass(frozen=True)
class Signal:
    name: str
    parameters: Payload


@dataclass(frozen=True)
class Property:
    name: str
    type: Type
    access: str


@dataclass(frozen=True)
class Service:
    name: str
    ordinal: int
    methods: tuple[Method, ...]
    signals: tuple[Signal, ...]
    properties: tuple[Property, ...]
    # Empty sections have observable legacy generation behavior.
    has_rpc_section: bool
    has_signal_section: bool
    has_property_section: bool


@dataclass(frozen=True)
class Enumeration:
    name: str
    members: tuple[tuple[str, int], ...]


@dataclass(frozen=True)
class Protocol:
    services: tuple[Service, ...]
    enumerations: tuple[Enumeration, ...] = ()
    inline_schema: str | None = None


def parse_type(value, location):
    if not isinstance(value, str):
        raise ValueError(f'{location}: expected a type name, got {value!r}')
    fixed = re.fullmatch(r'Data<(\d+)>', value)
    if fixed:
        return Type('Data', int(fixed[1]), value)
    return Type(value)


def payload(info, key, location):
    if key not in info:
        return Payload(PayloadKind.VOID)
    value = info[key]
    if isinstance(value, dict):
        return Payload(PayloadKind.FIELDS, tuple(
            Field(name, parse_type(type_name, f'{location}.{key}.{name}'), index)
            for index, (name, type_name) in enumerate(value.items())))
    if value == '__capnp__native__':
        return Payload(PayloadKind.NATIVE)
    if key == 'parameter':
        raise ValueError(f'{location}.parameter: expected a field table or __capnp__native__')
    return Payload(PayloadKind.DIRECT, type=parse_type(value, f'{location}.{key}'))


def build_protocol(data, source='<input>'):
    """Validate structural requirements without inventing new IDL restrictions."""
    from normalization import normalize
    if not isinstance(data.get('services'), dict):
        raise ValueError(f'{source}: services must be a table')
    for name, service in data['services'].items():
        location = f'{source}: services.{name}'
        if not isinstance(service, dict):
            raise ValueError(f'{location}: expected a table')
        for section in ('rpc', 'signal', 'properties'):
            if section in service and not isinstance(service[section], dict):
                raise ValueError(f'{location}.{section}: expected a table')
        for name, prop in service.get('properties', {}).items():
            if isinstance(prop, dict) and ('type' not in prop or not isinstance(prop.get('access'), str)):
                raise ValueError(f'{location}.properties.{name}: expected type and access')
    data = normalize(data)
    services = []
    for index, (name, service) in enumerate(data['services'].items()):
        location = f'{source}: services.{name}'
        methods, signals = [], []
        for ordinal, (method_name, info) in enumerate(service.get('rpc', {}).items()):
            loc = f'{location}.rpc.{method_name}'
            if not isinstance(info, dict):
                raise ValueError(f'{loc}: expected a table')
            methods.append(Method(method_name, ordinal, payload(info, 'parameter', loc), payload(info, 'returns', loc)))
        for signal_name, info in service.get('signal', {}).items():
            loc = f'{location}.signal.{signal_name}'
            if not isinstance(info, dict):
                raise ValueError(f'{loc}: expected a table')
            signals.append(Signal(signal_name, payload(info, 'parameter', loc)))
        properties = tuple(Property(key, parse_type(prop['type'], f'{location}.properties.{key}'), prop['access'])
                           for key, prop in service.get('properties', {}).items())
        services.append(Service(name, index, tuple(methods), tuple(signals), properties,
                                'rpc' in service, 'signal' in service, 'properties' in service))
    enums = data.get('enumerations', {})
    if not isinstance(enums, dict):
        raise ValueError(f'{source}: enumerations must be a table')
    for name, members in enums.items():
        if not isinstance(members, dict) or any(type(n) is not int for n in members.values()):
            raise ValueError(f'{source}: enumerations.{name}: expected integer ordinals')
    inline = data.get('capnp-inline')
    if inline is not None and not isinstance(inline, str):
        raise ValueError(f'{source}: capnp-inline must be text')
    return Protocol(tuple(services), tuple(Enumeration(n, tuple(m.items())) for n, m in enums.items()), inline)
