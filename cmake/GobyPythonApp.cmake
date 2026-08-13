# GobyPythonApp.cmake - convenience function for building Goby applications written in Python
#
# Provides:
#   add_goby_python_app(TARGET <name>
#     INTERFACE_YML <file>
#     MAIN <python file>
#     [PROTOS <proto_files>...]
#     [PYTHON_PROTOS <proto_files>...]
#     [PYTHON_PROTO_MODULES <modules>...]
#     [INCLUDES <headers>...]
#     [LINK_LIBRARIES <libs>...])
#
# Wraps goby_add_python_app() with the Python protobuf modules and a launcher in
# ${project_BIN_DIR}, so that a Python application is started like a C++ one.
# PROTOS are compiled for C++ and Python, PYTHON_PROTOS for Python only.
#
# Requires Goby built and installed with -Dbuild_python=ON.

function(add_goby_python_app)
  cmake_parse_arguments(args
    ""
    "TARGET;INTERFACE_YML;MAIN"
    "PROTOS;PYTHON_PROTOS;PYTHON_PROTO_MODULES;INCLUDES;LINK_LIBRARIES"
    ${ARGN})

  foreach(_required TARGET INTERFACE_YML MAIN)
    if(NOT args_${_required})
      message(FATAL_ERROR "add_goby_python_app: ${_required} is required")
    endif()
  endforeach()

  set(_out_dir "${CMAKE_CURRENT_BINARY_DIR}/${args_TARGET}")
  set(_python_proto_dir "${_out_dir}/proto")
  file(MAKE_DIRECTORY "${_python_proto_dir}")

  # --- protobuf ------------------------------------------------------------------------------
  # C++ for PROTOS only; the extension module compiles these in
  set(_cpp_proto_srcs)
  if(args_PROTOS)
    _goby_generate_protos(_cpp_proto_srcs "${_out_dir}" "${args_PROTOS}" "")
  endif()

  set(_import_flags)
  set(_seen_dirs)
  foreach(_dir ${CMAKE_CURRENT_SOURCE_DIR} ${project_INC_DIR} ${GOBY_PROTOBUF_IMPORT_DIRS})
    get_filename_component(_abs_dir "${_dir}" ABSOLUTE)
    if(NOT "${_abs_dir}" IN_LIST _seen_dirs)
      list(APPEND _seen_dirs "${_abs_dir}")
      list(APPEND _import_flags -I "${_abs_dir}")
    endif()
  endforeach()

  # Goby's own .proto files are absent here: they ship with the goby Python package, and
  # protobuf refuses to register the same descriptor file twice in one process
  set(_python_proto_outs)
  foreach(_proto ${args_PROTOS} ${args_PYTHON_PROTOS})
    get_filename_component(_abs_proto "${_proto}" ABSOLUTE)
    get_filename_component(_proto_we "${_abs_proto}" NAME_WE)

    # protoc mirrors the path of the .proto relative to its import directory, which is what
    # decides the Python module path (${project_INC_DIR}/messages/nav.proto -> messages.nav_pb2)
    set(_relative_out "${_proto_we}_pb2.py")
    foreach(_import_dir ${project_INC_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
      file(RELATIVE_PATH _relative "${_import_dir}" "${_abs_proto}")
      if(NOT _relative MATCHES "^\\.\\.")
        string(REGEX REPLACE "\\.proto$" "_pb2.py" _relative_out "${_relative}")
        break()
      endif()
    endforeach()

    set(_python_proto_out "${_python_proto_dir}/${_relative_out}")
    add_custom_command(
      OUTPUT "${_python_proto_out}"
      COMMAND protobuf::protoc
      ARGS --python_out "${_python_proto_dir}"
           ${_import_flags}
           "${_abs_proto}"
      DEPENDS "${_abs_proto}" protobuf::protoc
      COMMENT "Running protobuf compiler (python) on ${_proto}"
      VERBATIM)
    list(APPEND _python_proto_outs "${_python_proto_out}")
  endforeach()

  add_custom_target(${args_TARGET}_python_protos DEPENDS ${_python_proto_outs})

  # --- the application -----------------------------------------------------------------------
  goby_add_python_app(
    TARGET ${args_TARGET}
    INTERFACE_YML "${args_INTERFACE_YML}"
    OUTPUT_DIRECTORY "${_out_dir}"
    SOURCES ${_cpp_proto_srcs}
    INCLUDES ${args_INCLUDES}
    PROTO_MODULES ${args_PYTHON_PROTO_MODULES}
    LINK_LIBRARIES goby ${args_LINK_LIBRARIES})

  add_dependencies(${args_TARGET} ${args_TARGET}_python_protos)
  target_include_directories(${args_TARGET} PRIVATE "${_out_dir}" "${CMAKE_CURRENT_SOURCE_DIR}")

  # --- launcher ------------------------------------------------------------------------------
  get_filename_component(_main "${args_MAIN}" ABSOLUTE)
  set(_pythonpath "${${args_TARGET}_PYTHON_DIRECTORY}:${_python_proto_dir}")

  # against an uninstalled Goby these are in its source and build trees rather than on the
  # interpreter's path; both are empty once python3-goby3 is installed
  foreach(_goby_python_dir "${GOBY_PYTHON_SOURCE_DIR}" "${GOBY_PYTHON_PROTO_DIR}")
    if(_goby_python_dir AND IS_DIRECTORY "${_goby_python_dir}")
      set(_pythonpath "${_pythonpath}:${_goby_python_dir}")
    endif()
  endforeach()

  file(GENERATE
    OUTPUT "${project_BIN_DIR}/${args_TARGET}"
    CONTENT "#!/bin/sh
PYTHONPATH=\"${_pythonpath}\${PYTHONPATH:+:\$PYTHONPATH}\"
export PYTHONPATH
exec \"${Python3_EXECUTABLE}\" \"${_main}\" \"$@\"
"
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endfunction()
