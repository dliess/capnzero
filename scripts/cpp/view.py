"""Lower semantic values to the existing C++ renderer's private input format.

This compatibility boundary keeps the proven text renderers small to review.
The parser and other backends never consume this representation.
"""
from model import PayloadKind


def type_name(type_):
    return type_.source_spelling or (f'Data<{type_.size}>' if type_.size is not None else type_.name)


def add_payload(result, key, payload):
    if payload.kind == PayloadKind.FIELDS:
        result[key] = {f.name: type_name(f.type) for f in payload.fields}
    elif payload.kind == PayloadKind.DIRECT:
        result[key] = type_name(payload.type)
    elif payload.kind == PayloadKind.NATIVE:
        result[key] = '__capnp__native__'


def renderer_data(protocol):
    services = {}
    for service in protocol.services:
        result = {}
        if service.has_rpc_section:
            result['rpc'] = {}
            for method in service.methods:
                info = {}
                add_payload(info, 'parameter', method.parameters)
                add_payload(info, 'returns', method.returns)
                result['rpc'][method.name] = info
        if service.has_signal_section:
            result['signal'] = {}
            for signal in service.signals:
                info = {}
                add_payload(info, 'parameter', signal.parameters)
                result['signal'][signal.name] = info
        if service.has_property_section:
            result['properties'] = {p.name: {'type': type_name(p.type), 'access': p.access} for p in service.properties}
        services[service.name] = result
    return {'services': services}
