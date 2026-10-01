"""Shared Cap'n Proto wire schema rendering."""
import subprocess
from model import PayloadKind
from wire import (
    lowerfirst, create_rpc_service_id_capnp_enum, create_rpc_id_enum,
    create_capnp_rpc_parameter_type_str, create_capnp_rpc_return_type_str,
    create_capnp_signal_param_type_str, map_descr_type_to_capnp_type,
)


def schema_type(type_):
    if type_.size is not None:
        return 'Data'
    return map_descr_type_to_capnp_type(type_.name)


def fields_text(fields):
    return ''.join(f'\t{f.name} @{f.ordinal} :{schema_type(f.type)};\n' for f in fields)


def render(protocol, name, schema_id=None, *, cpp_namespace=True):
    # Preserve the legacy random ID unless the caller supplies an identity.
    if schema_id is None:
        schema_id = subprocess.check_output(['capnp', 'id']).decode('utf-8').rstrip()
    out = f'{schema_id};\n\n'
    if cpp_namespace:
        out += f'using Cxx = import "/capnp/c++.capnp";\n$Cxx.namespace("capnzero::{name}");\n\n'
    if protocol.inline_schema is not None:
        out += protocol.inline_schema
    out += 'enum ServiceId {\n'
    for service in protocol.services:
        out += f'\t{create_rpc_service_id_capnp_enum(service.name)} @{service.ordinal};\n'
    out += '}\n'
    for enum in protocol.enumerations:
        out += f'enum {enum.name} {{\n'
        out += ''.join(f'\t{lowerfirst(key)} @{number};\n' for key, number in enum.members)
        out += '}\n'
    for service in protocol.services:
        if service.has_rpc_section:
            out += 'enum ' + create_rpc_id_enum(service.name) + '{\n'
            out += ''.join(f'\t{method.name} @{method.ordinal};\n' for method in service.methods)
            out += '}\n'
    out += 'struct RpcCoord {\n\tserviceId @0 :UInt16;\n\trpcId @1 :UInt16;\n}\n'
    for service in protocol.services:
        for method in service.methods:
            if method.parameters.kind == PayloadKind.FIELDS:
                out += 'struct ' + create_capnp_rpc_parameter_type_str(service.name, method.name) + '{\n'
                out += fields_text(method.parameters.fields) + '}\n'
            if method.returns.kind in (PayloadKind.FIELDS, PayloadKind.DIRECT):
                out += 'struct ' + create_capnp_rpc_return_type_str(service.name, method.name) + ' {\n'
                if method.returns.kind == PayloadKind.FIELDS:
                    out += fields_text(method.returns.fields)
                else:
                    out += f'\tretParam @0 :{schema_type(method.returns.type)};\n'
                out += '}\n'
        for signal in service.signals:
            if signal.parameters.kind == PayloadKind.FIELDS:
                out += 'struct ' + create_capnp_signal_param_type_str(service.name, signal.name) + '{\n'
                out += fields_text(signal.parameters.fields) + '}\n'
    return out
