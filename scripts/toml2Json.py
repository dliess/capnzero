#!/usr/bin/python3

import toml
import json
import sys

descrfile = sys.argv[1]
data = toml.load(descrfile)
print(json.dumps(data, indent=2))