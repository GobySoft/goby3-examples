# GobyJulia.cmake
# Support for building Goby applications with a Julia interface
#
# This file is included when enable_julia_examples is ON. It:
#   1. Locates Goby.jl installed with goby (${GOBY_INCLUDE_DIR}/../share/goby/Goby.jl)
#   2. Locates JlCxx for C++/Julia interop
#   3. Adds the goby_julia_pkgs target that installs Goby.jl's Julia dependencies
#   4. Provides goby_generate_julia() to build the C++ side of a Goby-Julia app
#   5. Provides protobuf_generate_julia() / generate_julia_protos() to generate
#      the Julia protobuf bindings for the project's messages
#
# If any of the required pieces are missing, a warning is issued and
# enable_julia_examples is set to OFF so that the rest of the project still builds.

# Warn, disable the Julia examples, and stop processing this file
macro(_goby_julia_disable REASON)
    message(WARNING "Julia examples will not be built: ${REASON}")
    set(enable_julia_examples OFF)
    return()
endmacro()

set(_goby_julia_cxxwrap_hint
    "install CxxWrap.jl in Julia to enable:\n  julia -e 'import Pkg; Pkg.add(\"CxxWrap\")'")

if(NOT JULIA)
    _goby_julia_disable(
        "julia was not found on the system PATH (set JULIA to the julia executable).")
endif()

# -- Locate Goby.jl from the goby installation ---------------------------------
get_filename_component(GOBY_JULIA_SRC_DIR "${GOBY_INCLUDE_DIR}/../share/goby/Goby.jl" ABSOLUTE)

if(NOT EXISTS "${GOBY_JULIA_SRC_DIR}/Project.toml")
    _goby_julia_disable(
        "Goby.jl not found at ${GOBY_JULIA_SRC_DIR}.\nEnsure Goby is installed with Julia support (requires goby >= 3.3).")
endif()

message(STATUS "Found Goby.jl at ${GOBY_JULIA_SRC_DIR}")

set(GOBY_JULIA_DIR "${project_BUILD_DIR}/julia/Goby.jl")

# Copy Goby.jl source to the build directory so that Julia can write Manifest.toml
set(GOBY_JULIA_MANIFEST "${GOBY_JULIA_DIR}/Manifest.toml")
file(COPY "${GOBY_JULIA_SRC_DIR}/src" DESTINATION "${GOBY_JULIA_DIR}")
file(COPY "${GOBY_JULIA_SRC_DIR}/Project.toml" DESTINATION "${GOBY_JULIA_DIR}")

# -- Locate JlCxx (the C++ side of CxxWrap.jl) ---------------------------------
if(NOT DEFINED CXXWRAP_PREFIX)
    execute_process(
        COMMAND "${JULIA}" -e "using CxxWrap; println(CxxWrap.prefix_path())"
        OUTPUT_VARIABLE _CXXWRAP_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _CXXWRAP_RESULT
    )
    if(NOT _CXXWRAP_RESULT EQUAL 0)
        _goby_julia_disable(
            "could not determine the CxxWrap.jl prefix path; ${_goby_julia_cxxwrap_hint}")
    endif()
    set(CXXWRAP_PREFIX "${_CXXWRAP_PREFIX}" CACHE PATH "Prefix path for JlCxx from Julia")
    message(STATUS "JlCxx prefix: ${CXXWRAP_PREFIX}")
endif()

find_package(JlCxx PATHS "${CXXWRAP_PREFIX}" QUIET)

if(NOT JlCxx_FOUND)
    _goby_julia_disable(
        "JlCxx not found at ${CXXWRAP_PREFIX}; ${_goby_julia_cxxwrap_hint}")
endif()

get_target_property(JlCxx_location JlCxx::cxxwrap_julia LOCATION)
get_filename_component(JlCxx_location "${JlCxx_location}" DIRECTORY)
message(STATUS "Found JlCxx at ${JlCxx_location}")

# -- Julia package installation ------------------------------------------------
# Installs the Julia dependencies declared by Goby.jl's Project.toml. This is a
# single target (rather than a rule per application) so that Manifest.toml is
# never written by two concurrent julia processes during a parallel build.
add_custom_command(
    OUTPUT "${GOBY_JULIA_MANIFEST}"
    COMMAND "${JULIA}"
    ARGS --project="${GOBY_JULIA_DIR}" -L "${GOBY_JULIA_DIR}/src/pkg.jl" -e "'install_pkgs()'"
    COMMENT "Installing Julia packages for Goby.jl"
)

add_custom_target(goby_julia_pkgs DEPENDS "${GOBY_JULIA_MANIFEST}")

# -- GOBY_GENERATE_JULIA -------------------------------------------------------
# Generate the C++ shared library that bridges a Goby application to Julia.
#
# goby_generate_julia(OUTPUT_TARGET JULIA_OUT_DIR INTERFACE_YML CONFIG_PROTO
#                     [INCLUDE_HEADERS...])
#
#   OUTPUT_TARGET   - name of the shared library target (e.g. basic_julia_publisher)
#   JULIA_OUT_DIR   - directory where the .so is placed (alongside the Julia script)
#   INTERFACE_YML   - path to interface.yml describing the application
#   CONFIG_PROTO    - path to the application configuration .proto file
#   INCLUDE_HEADERS - additional headers (relative to include path) to #include
#                     in the generated C++ file (e.g. messages/nav.pb.h)
function(GOBY_GENERATE_JULIA OUTPUT_TARGET JULIA_OUT_DIR INTERFACE_YML CONFIG_PROTO)
    set(JULIA_CPP_OUT_DIR "${project_BUILD_DIR}/julia/c++")
    file(MAKE_DIRECTORY "${JULIA_CPP_OUT_DIR}")
    set(OUTPUT_CPP "${JULIA_CPP_OUT_DIR}/${OUTPUT_TARGET}.cpp")

    get_filename_component(ABS_INTERFACE_YML "${INTERFACE_YML}" ABSOLUTE)

    # Generate C++ protobuf bindings for the config proto
    set(_PROTO_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_TARGET}_proto")
    file(MAKE_DIRECTORY "${_PROTO_OUT_DIR}")
    _goby_generate_protos(_proto_generated "${_PROTO_OUT_DIR}" "${CONFIG_PROTO}" "")

    set(PROTO_SRCS)
    set(PROTO_HDRS)
    foreach(_f ${_proto_generated})
        if(_f MATCHES "\\.cc$")
            list(APPEND PROTO_SRCS "${_f}")
        else()
            list(APPEND PROTO_HDRS "${_f}")
        endif()
    endforeach()

    # Build the include list for the generated C++ file.
    # ARGN contains the extra headers (relative paths, e.g. "messages/nav.pb.h").
    # PROTO_HDRS contains absolute paths to the generated config proto headers.
    set(INCLUDE_STR "")
    foreach(_arg IN LISTS ARGN PROTO_HDRS)
        string(APPEND INCLUDE_STR "\"${_arg}\",")
    endforeach()

    # Generate the C++ wrapper from the interface YAML using Goby.jl's gen_goby.jl
    add_custom_command(
        OUTPUT "${OUTPUT_CPP}"
        DEPENDS "${ABS_INTERFACE_YML}" ${_proto_generated}
        COMMAND "${JULIA}"
        ARGS --project=${GOBY_JULIA_DIR}
             -L "${GOBY_JULIA_DIR}/src/gen_goby.jl"
             -e "'goby_gen_cpp(\"${ABS_INTERFACE_YML}\",\"${OUTPUT_CPP}\",[${INCLUDE_STR}])'"
        COMMENT "Generating Julia C++ wrapper for ${OUTPUT_TARGET}"
    )

    # Build the shared library
    add_library("${OUTPUT_TARGET}" SHARED "${OUTPUT_CPP}" ${PROTO_SRCS})

    # gen_goby.jl needs Goby.jl's dependencies (YAML.jl) installed first
    add_dependencies("${OUTPUT_TARGET}" goby_julia_pkgs)

    set_target_properties("${OUTPUT_TARGET}" PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${JULIA_OUT_DIR}"
        # the library is dlopen'ed by julia, which needs to find libcxxwrap_julia
        INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${JlCxx_location}"
    )

    target_include_directories("${OUTPUT_TARGET}" PRIVATE
        "${_PROTO_OUT_DIR}"
        "${project_INC_DIR}"
    )

    target_link_libraries("${OUTPUT_TARGET}"
        JlCxx::cxxwrap_julia
        goby
        goby_zeromq
    )
endfunction()

function(PROTOBUF_JULIA_INCLUDE_DIRS)
  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_JULIA_INCLUDE_DIRS() called without any directories")
    return()
  endif()

  foreach(DIR ${ARGN})
    set(project_julia_proto_includes "${project_julia_proto_includes},\"${DIR}\"" CACHE INTERNAL "Project Julia Proto Includes")
  endforeach()

endfunction()

unset(project_julia_protos CACHE)
unset(project_julia_proto_includes CACHE)
unset(project_julia_proto_depends CACHE)
unset(project_julia_proto_output CACHE)

# protobuf_generate_julia(package proto_files)
# - package: directory style package subdirectory (e.g. "protobuf" for package protobuf;)
# - proto_files: variable length listing of path to .proto file(s) to generate.
function(PROTOBUF_GENERATE_JULIA PACKAGE)
  if(NOT ARGN)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_JULIA() called without any proto files")
    return()
  endif(NOT ARGN)

  set(JULIA_OUT_DIR ${project_BUILD_DIR}/julia)
  file(MAKE_DIRECTORY ${JULIA_OUT_DIR})
  protobuf_julia_include_dirs(${CMAKE_CURRENT_SOURCE_DIR})

  foreach(FIL ${ARGN})

    # full file name (relative to current source directory)
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    # relative file name (need to do this in case an absolute path is passed in)
    STRING(REGEX REPLACE "^${CMAKE_CURRENT_SOURCE_DIR}/" "" REL_FIL ${ABS_FIL})
    # name without extension, e.g. for "foo.proto", ${FIL_WE} is "foo"
    get_filename_component(FIL_WE ${REL_FIL} NAME_WE)
    # relative directory to current source directory, e.g. for "dir/foo.proto", ${FIL_DIR} is "dir"
    get_filename_component(FIL_DIR ${REL_FIL} DIRECTORY BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    # ProtoBuf.jl writes one <name>_pb.jl per proto into a directory tree that
    # mirrors the proto's package (e.g. package goby3_examples.protobuf gives
    # goby3_examples/protobuf/foo_pb.jl), which we can't derive here without
    # parsing the .proto. Track the top-level module file instead: it is
    # generated for every package and is the file the Julia scripts include.
    set(PROTO_JL_OUT "${JULIA_OUT_DIR}/${PACKAGE}/${PACKAGE}.jl")

    set(project_julia_protos "${project_julia_protos},\"${FIL_WE}.proto\"" CACHE INTERNAL "Project Julia Protos")
    set(project_julia_proto_output "${project_julia_proto_output};${PROTO_JL_OUT}" CACHE INTERNAL "Project Julia Proto Outputs")
    set(project_julia_proto_depends "${project_julia_proto_depends};${CMAKE_CURRENT_SOURCE_DIR}/${FIL_WE}.proto" CACHE INTERNAL "Project Julia Proto Dependency list")
  endforeach()

endfunction()

# Run the actual generation of the proto files collected by PROTOBUF_GENERATE_JULIA at once (required to get modules correct with Protobuf.jl)
macro(generate_julia_protos)
  # remove empty item at the beginning of the list
  list(REMOVE_AT project_julia_proto_output 0)
  list(REMOVE_AT project_julia_proto_depends 0)
  # several protos may share a package, and thus a top-level module file
  list(REMOVE_DUPLICATES project_julia_proto_output)

  string(SUBSTRING "${project_julia_proto_includes}" 1 -1 project_julia_proto_includes)
  string(SUBSTRING "${project_julia_protos}" 1 -1 project_julia_protos)

  # gen_proto() writes this once it has generated all the bindings
  set(project_julia_proto_stamp "${project_BUILD_DIR}/julia/.protos.stamp")

  add_custom_command(
    OUTPUT ${project_julia_proto_output}
    BYPRODUCTS ${project_julia_proto_stamp}
    DEPENDS ${project_julia_proto_depends}
    COMMAND ${JULIA}
    ARGS --project=${GOBY_JULIA_DIR} -L ${GOBY_JULIA_DIR}/src/gen_goby.jl -e "'gen_proto([${project_julia_protos}],[${project_julia_proto_includes}],\"${project_BUILD_DIR}/julia\",\"${project_julia_proto_stamp}\")'"
    COMMENT "Running Julia protocol buffer compiler on all project protos"
  )
  add_custom_target(julia_build_protos ALL
    DEPENDS ${project_julia_proto_output}
  )
  # gen_goby.jl needs Goby.jl's dependencies (ProtoBuf.jl) installed first
  add_dependencies(julia_build_protos goby_julia_pkgs)

endmacro()
