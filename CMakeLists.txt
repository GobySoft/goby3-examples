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

# Import directories passed to protoc for all Goby targets (via GobyApp.cmake)
set(GOBY_PROTOBUF_IMPORT_DIRS
  ${DCCL_INCLUDE_DIR}
  ${GOBY_INCLUDE_DIR}
  ${project_INC_DIR}
)

include(GobyTarget)

# Julia examples – enabled by default if julia is found on the system
find_program(JULIA julia)
if(JULIA)
  set(_julia_examples_default ON)
else()
  set(_julia_examples_default OFF)
endif()
option(enable_julia_examples
  "Build Julia interprocess examples (requires julia, CxxWrap.jl, and JlCxx C++ headers)"
  ${_julia_examples_default})
if(enable_julia_examples)
  include(GobyJulia)
  protobuf_julia_include_dirs("${DCCL_INCLUDE_DIR}")
  protobuf_julia_include_dirs("${GOBY_INCLUDE_DIR}")
  protobuf_julia_include_dirs("${project_INC_DIR}")
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


if(enable_julia_examples)
  generate_julia_protos()
endif()
