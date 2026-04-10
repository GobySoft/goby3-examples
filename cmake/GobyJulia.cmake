# GobyJulia.cmake
# Support for building Goby applications with a Julia interface
#
# This file is included when enable_julia_examples is ON. It:
#   1. Locates Goby.jl (bundled with this repo or from the goby installation)
#   2. Installs Julia package dependencies for Goby.jl at build time
#   3. Locates JlCxx for C++/Julia interop
#   4. Provides GOBY_GENERATE_JULIA() to build the C++ side of a Goby-Julia app
#   5. Provides GOBY_GENERATE_JULIA_PROTO() to generate Julia protobuf bindings
#
# Prerequisites (installed by the user):
#   - julia executable in PATH
#   - Julia packages: CxxWrap (>=0.17.4), ProtoBuf (>=1.2.0), YAML (>=0.4.13), ThreadPools (>=2.1.1)
#   - JlCxx C++ library (installed alongside CxxWrap.jl)
#
# Usage:
#   # In a CMakeLists.txt:
#   goby_generate_julia(my_app ${MY_JULIA_DIR} interface.yml config.proto
#       messages/nav.pb.h messages/groups.h)
#   target_link_libraries(my_app goby3_example_messages)
#
#   goby_generate_julia_proto(nav.proto ${SRC_DIR}/messages ${JULIA_OUT_DIR})

# -- Locate Goby.jl source -----------------------------------------------------
# Prefer the copy installed with goby (${GOBY_INCLUDE_DIR}/../share/goby/Goby.jl)
# and fall back to the bundled copy in this repository.
get_filename_component(_goby_install_julia "${GOBY_INCLUDE_DIR}/../share/goby/Goby.jl" ABSOLUTE)

if(EXISTS "${_goby_install_julia}/Project.toml")
    set(GOBY_JULIA_SRC_DIR "${_goby_install_julia}")
    message(STATUS "Goby.jl found in goby installation: ${GOBY_JULIA_SRC_DIR}")
else()
    set(GOBY_JULIA_SRC_DIR "${CMAKE_SOURCE_DIR}/julia/Goby.jl")
    message(STATUS "Goby.jl not found in goby installation, using bundled copy: ${GOBY_JULIA_SRC_DIR}")
endif()

set(GOBY_JULIA_DIR "${project_BUILD_DIR}/julia/Goby.jl")

# Copy Goby.jl source to the build directory so that Julia can write Manifest.toml
file(COPY "${GOBY_JULIA_SRC_DIR}" DESTINATION "${project_BUILD_DIR}/julia")

# -- Install Julia packages at build time --------------------------------------
add_custom_command(
    OUTPUT "${GOBY_JULIA_DIR}/Manifest.toml"
    COMMAND "${JULIA}"
    ARGS --project="${GOBY_JULIA_DIR}" -L "${GOBY_JULIA_DIR}/src/pkg.jl" -e "install_pkgs()"
    COMMENT "Installing Julia packages for Goby.jl (CxxWrap, ProtoBuf, YAML, ThreadPools)"
)

add_custom_target(goby_julia_packages
    DEPENDS "${GOBY_JULIA_DIR}/Manifest.toml"
)

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
        message(WARNING
            "Could not determine CxxWrap.jl prefix path. "
            "Julia examples will not be built. "
            "To enable, install CxxWrap.jl in Julia:\n"
            "  julia -e 'import Pkg; Pkg.add(\"CxxWrap\")'")
        set(enable_julia_examples OFF CACHE BOOL "" FORCE)
        return()
    endif()
    set(CXXWRAP_PREFIX "${_CXXWRAP_PREFIX}" CACHE PATH "Prefix path for JlCxx from Julia")
    message(STATUS "JlCxx prefix: ${CXXWRAP_PREFIX}")
endif()

find_package(JlCxx PATHS "${CXXWRAP_PREFIX}" QUIET)

if(NOT JlCxx_FOUND)
    message(WARNING
        "JlCxx not found at ${CXXWRAP_PREFIX}. "
        "Julia examples will not be built. "
        "Install CxxWrap.jl in Julia to enable:\n"
        "  julia -e 'import Pkg; Pkg.add(\"CxxWrap\")'")
    set(enable_julia_examples OFF CACHE BOOL "" FORCE)
    return()
endif()

get_target_property(JlCxx_location JlCxx::cxxwrap_julia LOCATION)
get_filename_component(JlCxx_location "${JlCxx_location}" DIRECTORY)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${JlCxx_location}")
message(STATUS "Found JlCxx at ${JlCxx_location}")

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
        DEPENDS "${GOBY_JULIA_DIR}/Manifest.toml" "${ABS_INTERFACE_YML}" ${_proto_generated}
        COMMAND "${JULIA}"
        ARGS --project="${GOBY_JULIA_DIR}"
             -L "${GOBY_JULIA_DIR}/src/gen_goby.jl"
             -e "goby_gen_cpp(\"${ABS_INTERFACE_YML}\",\"${OUTPUT_CPP}\",[${INCLUDE_STR}])"
        COMMENT "Generating Julia C++ wrapper for ${OUTPUT_TARGET}"
    )

    # Build the shared library
    add_library("${OUTPUT_TARGET}" SHARED "${OUTPUT_CPP}" ${PROTO_SRCS})

    set_target_properties("${OUTPUT_TARGET}" PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${JULIA_OUT_DIR}"
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

# -- GOBY_GENERATE_JULIA_PROTO -------------------------------------------------
# Generate Julia protobuf bindings from a .proto file using ProtoBuf.jl.
#
# goby_generate_julia_proto(PROTO_FILE INCLUDE_DIR JULIA_OUT_DIR)
#
#   PROTO_FILE    - path to the .proto file (e.g. src/messages/nav.proto)
#   INCLUDE_DIR   - directory to pass as the proto search path to protojl
#   JULIA_OUT_DIR - output directory for the generated Julia files
#
# The generated files are placed in a subdirectory named after the proto package.
# Use add_dependencies(<target> julia_proto_<basename>) to depend on the output.
function(GOBY_GENERATE_JULIA_PROTO PROTO_FILE INCLUDE_DIR JULIA_OUT_DIR)
    get_filename_component(_proto_name "${PROTO_FILE}" NAME)
    get_filename_component(_proto_we   "${PROTO_FILE}" NAME_WE)

    file(MAKE_DIRECTORY "${JULIA_OUT_DIR}")

    # We cannot know the exact output filename ahead of time (it depends on the
    # package name declared inside the .proto file), so we use a stamp file to
    # track whether the generation has run.
    set(_stamp "${JULIA_OUT_DIR}/.${_proto_we}_pb.stamp")

    add_custom_command(
        OUTPUT "${_stamp}"
        DEPENDS "${PROTO_FILE}" "${GOBY_JULIA_DIR}/Manifest.toml"
        COMMAND "${JULIA}"
        ARGS --project="${GOBY_JULIA_DIR}"
             -e "using ProtoBuf; \
                 protojl(\"${_proto_name}\", [\"${INCLUDE_DIR}\"], \"${JULIA_OUT_DIR}\", \
                         always_use_modules=false); \
                 open(\"${_stamp}\", \"w\") do io; write(io, \"done\"); end"
        COMMENT "Generating Julia protobuf bindings for ${_proto_name}"
    )

    add_custom_target("julia_proto_${_proto_we}"
        DEPENDS "${_stamp}"
    )
endfunction()
