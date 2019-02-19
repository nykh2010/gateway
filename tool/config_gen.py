#!/usr/bin/env python
from configparser import ConfigParser
import os

FILE_NAME = "web"
PATH = os.path.join(os.path.dirname(__file__), "../config/%s.ini" % (FILE_NAME))


cf = ConfigParser()

cf.add_section(FILE_NAME)
cf.set(FILE_NAME, "host", '"localhost"')
cf.set(FILE_NAME, "port", "8000")

with open(PATH, "w") as f:
    cf.write(f)
