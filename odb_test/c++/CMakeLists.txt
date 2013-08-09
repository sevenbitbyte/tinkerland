cmake_minimum_required(VERSION 2.8.8)

project(odb_test)

include(ODB.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


set(ODB_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/odb_gen)
file(MAKE_DIRECTORY ${ODB_OUT_DIR})

SET(SOURCES
	person.cpp
	person_driver.cpp
	${ODB_OUT_DIR}/person_odb.cpp
)


include_directories(
	..
	${ODB_OUT_DIR}
)

set(ODB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/person.hxx)
odb_compile(HEADER ${ODB_HEADERS} OUTPUT_DIRECTORY ${ODB_OUT_DIR} DB sqlite GENERATE_QUERY GENERATE_SCHEMA SCHEMA_FORMAT embedded SCHEMA_NAME v1 TABLE_PREFIX schema_v1_)


add_executable(odb_test ${SOURCES})

target_link_libraries(odb_test -lodb -lodb-sqlite)
