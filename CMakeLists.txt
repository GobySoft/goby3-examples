cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(goby3-examples C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")

if(((NOT DEFINED CMAKE_CXX_STANDARD) OR ("${CMAKE_CXX_STANDARD}" STREQUAL "03") OR ("${CMAKE_CXX_STANDARD}" STREQUAL "11") OR ("${CMAKE_CXX_STANDARD}" STREQUAL "14")))
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  message ("Setting CMAKE_CXX_STANDARD to 17 as required by Goby. If you wish to use a newer standard, set CMAKE_CXX_STANDARD directly")
endif()

find_package(ProtobufLocal REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

find_package(DCCL REQUIRED)
get_target_property(DCCL_INCLUDE_DIR dccl INTERFACE_INCLUDE_DIRECTORIES)

message("Using DCCL in ${DCCL_INCLUDE_DIR}")
include_directories("${DCCL_INCLUDE_DIR}")
protobuf_include_dirs("${DCCL_INCLUDE_DIR}")

find_package(GOBY 3.0 REQUIRED zeromq)

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

option(export_goby_interfaces OFF "Build Goby Publish/Subscribe interface files (YML) from code")

if(export_goby_interfaces)
  include(GobyClangTool)
  set(YML_OUT_DIR ${project_SHARE_DIR}/interfaces)
endif()

add_subdirectory(src)

if(export_goby_interfaces)
  generate_interfaces_figure(launch/interprocess/gps_driver_deployment.yml ${YML_OUT_DIR} gps_driver_interfaces.pdf "")  
  generate_interfaces_figure(launch/interprocess/basic_publisher_subscriber_deployment.yml ${YML_OUT_DIR} basic_interprocess_interfaces.pdf "")
  generate_interfaces_figure(launch/interprocess/state_machine_deployment.yml ${YML_OUT_DIR} state_machine_interfaces.pdf "")
  generate_interfaces_figure(launch/interprocess/hold_deployment.yml ${YML_OUT_DIR} hold_interfaces.pdf "")
  generate_interfaces_figure(launch/interprocess/json_deployment.yml ${YML_OUT_DIR} json_interfaces.pdf "")
  generate_interfaces_figure(launch/interthread/basic_interthread_deployment.yml ${YML_OUT_DIR} basic_interthread_interfaces.pdf "")
  generate_interfaces_figure(launch/intervehicle/basic/basic_intervehicle_deployment.yml ${YML_OUT_DIR} basic_intervehicle_interfaces.pdf "")
  generate_interfaces_figure(launch/interprocess/mavlink_deployment.yml ${YML_OUT_DIR} mavlink_interfaces.pdf "")
  
endif()



## configure to build/include
file(GLOB_RECURSE INCLUDE_FILES RELATIVE ${project_SRC_DIR} 
  src/messages/*.proto
  src/messages/*.h
  )
foreach(I ${INCLUDE_FILES})
  configure_file(${project_SRC_DIR}/${I} ${project_INC_DIR}/${I} @ONLY)
endforeach()
