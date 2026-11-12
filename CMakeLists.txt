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

find_package(GOBY 3.0 REQUIRED zeromq OPTIONAL_COMPONENTS zenoh)

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

option(enable_tests "Register the goby pub/sub test harnesses in test/ with ctest" OFF)
option(build_test_figures "Render each test harness's plan to SVG with 'goby test visualize' and Graphviz dot (requires graphviz)" OFF)
if(enable_tests)
  enable_testing()
endif()
if(enable_tests OR build_test_figures)
  add_subdirectory(test)
endif()

# 'goby interface' (visualize, validate, launch_check) comes with newer versions of goby, along
# with goby_validate_interfaces()
if(export_goby_interfaces AND COMMAND goby_validate_interfaces AND GOBY_EXECUTABLE)
  # the deployments' run: entries use launch keys (args, config, hold, enabled), which 'goby
  # interface' reads only from the version that added 'goby interface launch'
  set(probe ${CMAKE_CURRENT_BINARY_DIR}/goby_interface_launch_keys_probe.yml)
  file(WRITE ${probe} "deployment: probe\nplatforms:\n  - name: probe\n    run:\n      - { process: probe, exec: \"true\", args: -v, enabled: false }\n")
  execute_process(COMMAND ${GOBY_EXECUTABLE} interface validate ${probe}
    RESULT_VARIABLE probe_result OUTPUT_QUIET ERROR_QUIET)
  if(probe_result EQUAL 0)
    set(goby_interface_tool_found ON)
  else()
    message(STATUS "This goby's 'goby interface' does not read launch keys in run: entries (it predates 'goby interface launch'): deployment figures and checks are skipped")
  endif()
elseif(export_goby_interfaces)
  message(STATUS "This goby has no 'goby interface' tool: deployment figures and checks are skipped")
endif()

option(build_interface_figures_d2 "With export_goby_interfaces, also draw each deployment as a D2 diagram (rendered to SVG when d2 is found)" OFF)
if(goby_interface_tool_found AND build_interface_figures_d2)
  find_program(D2_EXECUTABLE d2)
  if(NOT D2_EXECUTABLE)
    message(STATUS "d2 (https://d2lang.com) was not found: the D2 figures are written but not rendered")
  endif()
endif()

# deployment name, file, and (for the launch_check tests) launch files as <platform>=<file>
set(deployments
  "basic_interprocess launch/interprocess/zeromq/basic_publisher_subscriber_deployment.yml vehicle1=basic_publisher_subscriber.launch"
  "gps_driver launch/interprocess/zeromq/gps_driver_deployment.yml auv0=gps_driver.launch"
  "hold launch/interprocess/zeromq/hold_deployment.yml vehicle1=hold.launch"
  "json launch/interprocess/zeromq/json_deployment.yml vehicle1=json.launch"
  "state_machine launch/interprocess/zeromq/state_machine_deployment.yml example=state_machine.launch"
  "mavlink launch/interprocess/zeromq/mavlink_deployment.yml auv0=mavlink.launch"
  "basic_udpm launch/interprocess/udpm/basic_publisher_subscriber_deployment.yml vehicle1=basic_publisher_subscriber.launch"
  "basic_interthread launch/interthread/basic_interthread_deployment.yml vehicle1=basic_interthread.launch"
  "basic_intervehicle launch/intervehicle/basic/basic_intervehicle_deployment.yml vehicle1=basic_vehicle1.launch vehicle2=basic_vehicle2.launch")
if(TARGET goby3_example_basic_zenoh_interprocess_publisher)
  list(APPEND deployments
    "basic_zenoh launch/interprocess/zenoh/basic_publisher_subscriber_deployment.yml vehicle1=basic_publisher_subscriber.launch")
endif()

if(goby_interface_tool_found)
  foreach(entry ${deployments})
    string(REPLACE " " ";" parts "${entry}")
    list(GET parts 0 name)
    list(GET parts 1 file)
    list(REMOVE_AT parts 0 1)
    generate_interfaces_figure(DEPLOYMENT ${file} NAME ${name}_interfaces)

    if(enable_tests)
      # 'goby interface validate --strict': a required subscription with no publisher fails
      goby_validate_interfaces(NAME goby3_example_interfaces_${name}
        DEPLOYMENT_YAML ${CMAKE_CURRENT_SOURCE_DIR}/${file}
        INTERFACE_DIRS ${YML_OUT_DIR})
      # 'goby interface launch_check': the launch file runs what the deployment lists
      get_filename_component(launch_dir ${CMAKE_CURRENT_SOURCE_DIR}/${file} DIRECTORY)
      foreach(launch ${parts})
        string(REPLACE "=" ";" launch_parts ${launch})
        list(GET launch_parts 0 platform)
        list(GET launch_parts 1 launch_file)
        add_test(NAME goby3_example_launch_${name}_${platform}
          COMMAND ${GOBY_EXECUTABLE} interface launch_check ${CMAKE_CURRENT_SOURCE_DIR}/${file}
                  ${launch_dir}/${launch_file} --platform ${platform} -I ${YML_OUT_DIR})
      endforeach()
    endif()
  endforeach()
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
