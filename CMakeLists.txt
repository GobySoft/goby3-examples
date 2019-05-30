project(goby3-examples C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")

cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)
# This policy determines whether or not to follow symlinks encountered during a FILE GLOB_RECURSE cal
cmake_policy(SET CMP0009 NEW)

# require C++14
if(CMAKE_VERSION VERSION_LESS 3.1)
  add_definitions(-std=c++14)
else()
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

find_package(ProtobufLocal REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

find_package(DCCL REQUIRED)
get_target_property(DCCL_INCLUDE_DIR dccl INTERFACE_INCLUDE_DIRECTORIES)

message("Using DCCL in ${DCCL_INCLUDE_DIR}")
include_directories("${DCCL_INCLUDE_DIR}")
protobuf_include_dirs("${DCCL_INCLUDE_DIR}")

find_package(GOBY 3 REQUIRED)
get_target_property(GOBY_INCLUDE_DIR goby INTERFACE_INCLUDE_DIRECTORIES)

message("Using Goby in ${GOBY_INCLUDE_DIR}")
include_directories("${GOBY_INCLUDE_DIR}")
protobuf_include_dirs("${GOBY_INCLUDE_DIR}")

# create variables for various directories
get_filename_component(project_SRC_DIR src ABSOLUTE)
get_filename_component(project_BUILD_DIR ${CMAKE_BINARY_DIR} ABSOLUTE)
get_filename_component(project_SCRIPTS_DIR scripts ABSOLUTE)

# fetch all the local directories for generated code
get_filename_component(project_LIB_DIR ${project_BUILD_DIR}/lib ABSOLUTE)
get_filename_component(project_BIN_DIR ${project_BUILD_DIR}/bin ABSOLUTE)
get_filename_component(project_INC_DIR ${project_BUILD_DIR}/include ABSOLUTE)
get_filename_component(project_SHARE_DIR ${project_BUILD_DIR}/share ABSOLUTE)

## set the cmake defaults for libraries and binaries
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${project_LIB_DIR} CACHE PATH 
  "Output directory for the dynamic libraries" )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${project_BIN_DIR} CACHE PATH
  "Output directory for the binaries" )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${project_LIB_DIR} CACHE PATH 
  "Output directory for the static libraries (archives)" )

include_directories(${project_INC_DIR})
protobuf_include_dirs(${project_INC_DIR})

add_subdirectory(src)

## configure to build/include
file(GLOB_RECURSE INCLUDE_FILES RELATIVE ${project_SRC_DIR} 
  src/messages/*.proto
  src/messages/*.h
  )
foreach(I ${INCLUDE_FILES})
  configure_file(${project_SRC_DIR}/${I} ${project_INC_DIR}/${I} @ONLY)
endforeach()

