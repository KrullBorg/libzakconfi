#!/usr/bin/env python2

from gi.repository import GLib
from gi.repository import Confi

# Create a new object
confi = Confi.Confi.new("PostgreSQL://postgres:postgres@HOST=localhost;DB_NAME=confi", "Default", None, False)

print("Properties of a Confi object: ")
print(dir(confi.props))

value = confi.path_get_value ("folder/key1/key1_2")

print("Value from key \"folder/key1/key1_2\": " + str(value))

key = confi.path_get_confi_key ("folder/key1/key1_2")
print("Path of confi key \"folder/key1/key1_2\": " + str(key.path))
print("Value from confi key \"folder/key1/key1_2\": " + str(key.value))

list = confi.get_configs_list ("PostgreSQL://postgres:postgres@HOST=localhost;DB_NAME=confi");
print("The name of configuration 0: " + str(list[0].get_property("name")))

confi.set_root ("key2")

value = confi.path_get_value ("key2-1")

print("Value from key \"key2-1\": " + str(value))

confi.set_root ("/")
confi.add_key ("folder", "key3");
confi.add_key_with_value ("folder/key3", "key3-1", "value of key3-1 added programmatically")

confi.set_root ("folder/key3")

value = confi.path_get_value ("key3-1")

print("Value from key \"key3-1\": " + str(value))
