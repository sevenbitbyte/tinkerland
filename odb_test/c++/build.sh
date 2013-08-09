#!/bin/bash

odb --database sqlite --generate-schema --schema-format embedded --schema-name v1 --table-prefix schema_v1_ person.hxx

g++ -c person.cpp -ggdb
g++ -c person_driver.cpp -ggdb
g++ -c person-odb.cxx -ggdb

g++ -o driver person.o person_driver.o person-odb.o -lodb-sqlite -lodb -ggdb
