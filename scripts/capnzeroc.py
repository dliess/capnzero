#!/usr/bin/python3

import toml
import getopt
import sys
import os.path
import re
import global_types
from common import *
from capnp_file import *
from client_transport_h import *
from client_transport_inl import *
from client_h import *
from client_inl import *
from client_cpp import *
from server_h import *
from server_cpp import *
from rpc_interface_headers import *
from qobject_client_h import *
from qobject_client_cpp import *

def expand_properties(data):
    for service_name in data["services"]:
        service = data["services"][service_name]
        if "properties" in service:
            for key, descr in service["properties"].items():
                if not isinstance(descr, dict):
                    propertyType = service["properties"][key]
                    service["properties"][key] = { 'type' :  propertyType, 'access' : "read-write" }

            for key, descr in service["properties"].items():
                if not "signal" in service:
                    service["signal"] = dict()
                service["signal"].update( { "{}Changed".format(lowerfirst(key)) : { "parameter" : { 'val' : descr["type"] } } } )

                if "-write" in descr["access"]:
                    if not "rpc" in service:
                        service["rpc"] = dict()
                    service["rpc"].update( { "set{}".format(upperfirst(key)) : { "parameter" : { 'val' : descr["type"] } } } )
                if "-toggle" in descr["access"]:
                    if not "rpc" in service:
                        service["rpc"] = dict()
                    service["rpc"].update( { "toggle{}".format(upperfirst(key)) : {} } )


outdir="undefined"
descrfile="undefined"
options, remainder = getopt.getopt(sys.argv[1:], ['o:d:'], ['outdir=', 'descrfile='])
for opt, arg in options:
    if opt in ('-o', '--outdir'):
        outdir = arg
    elif opt in ('-d', '--descrfile'):
        descrfile = arg

file_we = os.path.splitext(os.path.basename(descrfile))[0]

print("outdir: " + outdir)
print("descrfile: " + descrfile)
print("file_we: " + file_we)

from pathlib import Path
Path(outdir).mkdir(parents=True, exist_ok=True)

capnp_file = outdir + "/" + file_we + ".capnp"
client_transport_h_file = outdir + "/" + file_we + "_ClientTransport.h"
client_transport_inl_file = outdir + "/" + file_we + "_ClientTransport.inl"
client_h_file = outdir + "/" + file_we + "_Client.h"
client_inl_file = outdir + "/" + file_we + "_Client.inl"
client_cpp_file = outdir + "/" + file_we + "_Client.cpp"
server_h_file = outdir + "/" + file_we + "_Server.h"
server_cpp_file = outdir + "/" + file_we + "_Server.cpp"
qobject_client_h_file = outdir + "/" + file_we + "_QObjectClient.h"
qobject_client_cpp_file = outdir + "/" + file_we + "_QObjectClient.cpp"

data = toml.load(descrfile)
global_types.init()
# add keys if they don't exist
if "enumerations" in data:
    global_types.enumerations = data["enumerations"]

expand_properties(data)

with open(capnp_file, 'w') as open_file:
    open_file.write(create_capnp_file_content_str(data, file_we))

with open(client_transport_h_file, 'w') as open_file:
    open_file.write(create_capnzero_client_transport_file_h_content_str(data, file_we))

with open(client_transport_inl_file, 'w') as open_file:
    open_file.write(create_capnzero_client_transport_file_inl_content_str(data, file_we))

with open(client_h_file, 'w') as open_file:
    open_file.write(create_capnzero_client_file_h_content_str(data, file_we))

with open(client_inl_file, 'w') as open_file:
    open_file.write(create_capnzero_client_file_inl_content_str(data, file_we))

with open(client_cpp_file, 'w') as open_file:
    open_file.write(create_capnzero_client_file_cpp_content_str(data, file_we))

for service_name in data["services"]:
    service = data["services"][service_name]
    if "rpc" in service:
        rpc_if_filename_we = file_we + create_member_cb_if_type(service_name)
        with open(outdir + "/" + rpc_if_filename_we + ".h", 'w') as open_file:
            open_file.write(create_capnzero_cbif_h_content_str(service_name, service["rpc"], rpc_if_filename_we, file_we))

with open(server_h_file, 'w') as open_file:
    open_file.write(create_capnzero_server_file_h_content_str(data, file_we))

with open(server_cpp_file, 'w') as open_file:
    open_file.write(create_capnzero_server_file_cpp_content_str(data, file_we))


with open(qobject_client_h_file, 'w') as open_file:
    open_file.write(create_capnzero_qobject_client_file_h_content_str(data, file_we))

with open(qobject_client_cpp_file, 'w') as open_file:
    open_file.write(create_capnzero_qobject_client_file_cpp_content_str(data, file_we))
