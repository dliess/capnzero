"""Render the common protocol through the established C++ text emitters."""
from .common import *
from .client_transport_h import *
from .client_transport_inl import *
from .client_h import *
from .client_inl import *
from .client_cpp import *
from .server_h import *
from .server_cpp import *
from .rpc_interface_headers import *
from .qobject_client_h import *
from .qobject_client_cpp import *
from .view import renderer_data

def render(protocol, file_we):
    data = renderer_data(protocol)
    enumerations = {e.name: dict(e.members) for e in protocol.enumerations}
    files = {}
    client_transport_h_file = file_we + "_ClientTransport.h"
    client_transport_inl_file = file_we + "_ClientTransport.inl"
    client_h_file = file_we + "_Client.h"
    client_inl_file = file_we + "_Client.inl"
    client_cpp_file = file_we + "_Client.cpp"
    server_h_file = file_we + "_Server.h"
    server_cpp_file = file_we + "_Server.cpp"
    qobject_client_h_file = file_we + "_QObjectClient.h"
    qobject_client_cpp_file = file_we + "_QObjectClient.cpp"
    qobject_wc_client_cpp_file = file_we + "_QObjectWcClient.cpp"


    files[client_transport_h_file] = create_capnzero_client_transport_file_h_content_str(data, file_we)

    files[client_transport_inl_file] = create_capnzero_client_transport_file_inl_content_str(data, file_we)

    files[client_h_file] = create_capnzero_client_file_h_content_str(data, file_we, enumerations=enumerations)

    files[client_inl_file] = create_capnzero_client_file_inl_content_str(data, file_we, enumerations=enumerations)

    files[client_cpp_file] = create_capnzero_client_file_cpp_content_str(data, file_we, enumerations=enumerations)

    for service_name in data["services"]:
        service = data["services"][service_name]
        if "rpc" in service:
            rpc_if_filename_we = file_we + create_member_cb_if_type(service_name)
            rpc_if_filename = rpc_if_filename_we + ".h"
            files[rpc_if_filename] = create_capnzero_cbif_h_content_str(service_name, service["rpc"], rpc_if_filename_we, file_we, enumerations=enumerations)

    files[server_h_file] = create_capnzero_server_file_h_content_str(data, file_we, enumerations=enumerations)

    files[server_cpp_file] = create_capnzero_server_file_cpp_content_str(data, file_we, enumerations=enumerations)


    files[qobject_client_h_file] = create_capnzero_qobject_client_file_h_content_str(data, file_we, enumerations=enumerations)

    files[qobject_client_cpp_file] = create_capnzero_qobject_client_file_cpp_content_str(data, file_we, webchannel_support = False, enumerations=enumerations)

    files[qobject_wc_client_cpp_file] = create_capnzero_qobject_client_file_cpp_content_str(data, file_we, webchannel_support = True, enumerations=enumerations)

    return files
