"""Legacy property lowering; declaration order is part of the protocol."""
from copy import deepcopy

def upperfirst(value):
    return value[:1].upper() + value[1:]

def lowerfirst(value):
    return value[:1].lower() + value[1:]

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


def normalize(data):
    result = deepcopy(data)
    expand_properties(result)
    return result
