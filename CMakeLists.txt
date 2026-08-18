cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(goby3-examples C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

if(NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 17)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  message("Setting CMAKE_CXX_STANDARD to 17 as required by Goby. If you wish to use a newer standard, set CMAKE_CXX_STANDARD directly")
endif()

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

find_package(DCCL REQUIRED)
get_target_property(DCCL_INCLUDE_DIR dccl INTERFACE_INCLUDE_DIRECTORIES)

message("Using DCCL in ${DCCL_INCLUDE_DIR}")
include_directories("${DCCL_INCLUDE_DIR}")

find_package(GOBY 3.0 REQUIRED zeromq)

include_directories("${GOBY_INCLUDE_DIR}")

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

# Import directories passed to protoc for all Goby targets. goby_add_app() and
# goby_add_library() come from Goby's GobyTarget.cmake, included by goby-config.cmake.
set(GOBY_PROTOBUF_IMPORT_DIRS
  ${DCCL_INCLUDE_DIR}
  ${GOBY_INCLUDE_DIR}
  ${project_INC_DIR}
)

## examples written in Python, which need Goby built with -Dbuild_python=ON
# (goby-config.cmake includes GobyPython.cmake, which provides goby_add_python_app and looks for
# pybind11 and the goby_gen_cpp generator)
set(PYTHON_DOC_STRING "Build the examples written in Python (requires Goby built with -Dbuild_python=ON)")
if(COMMAND goby_add_python_app AND pybind11_FOUND AND GOBY_GEN_CPP_COMMAND)
  option(build_python ${PYTHON_DOC_STRING} ON)
else()
  option(build_python ${PYTHON_DOC_STRING} OFF)
  message(">> setting build_python to OFF ... if you need this functionality: 1) build and install Goby with -Dbuild_python=ON; 2) run cmake -Dbuild_python=ON")
endif()

# not in the branch above: build_python can also be set to ON explicitly, and then the Goby side
# has to be reported as missing rather than failing later on an undefined command
if(build_python)
  if(NOT COMMAND goby_add_python_app)
    message(FATAL_ERROR
      "build_python=ON but Goby does not provide goby_add_python_app(). Build Goby with "
      "-Dbuild_python=ON and re-run cmake there, so that GobyPython.cmake is written next to "
      "goby-config.cmake, then point this project at it with -DGOBY_DIR=/path/to/goby3/build.")
  endif()
  if(NOT pybind11_FOUND)
    message(FATAL_ERROR "build_python=ON but pybind11 was not found. Install pybind11-dev (or pip install pybind11).")
  endif()
  if(NOT GOBY_GEN_CPP_COMMAND)
    message(FATAL_ERROR
      "build_python=ON but the goby_gen_cpp generator was not found. Install the goby Python "
      "package (python3-goby3), or set -DGOBY_PYTHON_SOURCE_DIR=/path/to/goby3/src/python.")
  endif()
endif()

## examples written in Julia, which need julia and CxxWrap.jl
include(GobyJulia)
set(JULIA_DOC_STRING "Build the examples written in Julia (requires julia and CxxWrap.jl)")
if(GOBY_JULIA_FOUND)
  option(enable_julia_examples ${JULIA_DOC_STRING} ON)
else()
  option(enable_julia_examples ${JULIA_DOC_STRING} OFF)
  message(">> setting enable_julia_examples to OFF ... ${GOBY_JULIA_NOT_FOUND_REASON}")
endif()

if(enable_julia_examples)
  if(NOT GOBY_JULIA_FOUND)
    message(FATAL_ERROR "enable_julia_examples=ON but ${GOBY_JULIA_NOT_FOUND_REASON}")
  endif()
  goby_julia_proto_include_dirs("${DCCL_INCLUDE_DIR}" "${GOBY_INCLUDE_DIR}" "${project_INC_DIR}")
endif()

option(export_goby_interfaces OFF "Build Goby Publish/Subscribe interface files (YML) from code")

if(export_goby_interfaces)
  include(GobyClangTool)
  set(YML_OUT_DIR ${project_SHARE_DIR}/interfaces)
endif()

add_subdirectory(src)

if(export_goby_interfaces)
  generate_interfaces_figure(INTERFACE_YML launch/interprocess/zeromq/gps_driver_deployment.yml YML_OUT_DIR ${YML_OUT_DIR} OUTPUT_IMAGE gps_driver_interfaces.pdf)  
  generate_interfaces_figure(INTERFACE_YML launch/interprocess/zeromq/basic_publisher_subscriber_deployment.yml YML_OUT_DIR ${YML_OUT_DIR} OUTPUT_IMAGE basic_interprocess_interfaces.pdf)
  generate_interfaces_figure(INTERFACE_YML launch/interprocess/zeromq/state_machine_deployment.yml YML_OUT_DIR ${YML_OUT_DIR} OUTPUT_IMAGE state_machine_interfaces.pdf)
  generate_interfaces_figure(INTERFACE_YML launch/interprocess/zeromq/hold_deployment.yml YML_OUT_DIR ${YML_OUT_DIR} OUTPUT_IMAGE hold_interfaces.pdf)
  generate_interfaces_figure(INTERFACE_YML launch/interprocess/zeromq/json_deployment.yml YML_OUT_DIR ${YML_OUT_DIR} OUTPUT_IMAGE json_interfaces.pdf)
  generate_interfaces_figure(INTERFACE_YML launch/interthread/basic_interthread_deployment.yml YML_OUT_DIR ${YML_OUT_DIR} OUTPUT_IMAGE basic_interthread_interfaces.pdf)
  generate_interfaces_figure(INTERFACE_YML launch/intervehicle/basic/basic_intervehicle_deployment.yml YML_OUT_DIR ${YML_OUT_DIR} OUTPUT_IMAGE basic_intervehicle_interfaces.pdf)
  generate_interfaces_figure(INTERFACE_YML launch/interprocess/zeromq/mavlink_deployment.yml YML_OUT_DIR ${YML_OUT_DIR} OUTPUT_IMAGE mavlink_interfaces.pdf)
  
endif()



## configure to build/include
file(GLOB_RECURSE INCLUDE_FILES RELATIVE ${project_SRC_DIR} 
  src/messages/*.proto
  src/messages/*.h
  )
foreach(I ${INCLUDE_FILES})
  configure_file(${project_SRC_DIR}/${I} ${project_INC_DIR}/${I} @ONLY)
endforeach()

# must run after all goby_add_julia_protos() calls have been collected
if(enable_julia_examples AND CMAKE_VERSION VERSION_LESS 3.19)
  goby_generate_julia_protos()
endif()
