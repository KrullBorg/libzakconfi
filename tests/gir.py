#!/usr/bin/env python2

from gi.repository import Confi

# Create a new object
confi = Confi.Confi.new("PostgreSQL://postgres:postgres@HOST=localhost;DB_NAME=confi", "Default", None, False)

value = confi.path_get_value ("folder/key1/key1_2");

print(value)