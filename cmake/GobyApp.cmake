# GobyApp.cmake - convenience functions for building Goby application targets
#
# Provides:
#   add_goby_executable(TARGET <name>
#     SOURCES <files>...
#     [PROTOS <proto_files>...]
#     [LINK_LIBRARIES <libs>...]
#     [PROTO_IMPORT_DIRS <dirs>...]
#     [PROTOC_OUT_DIR <dir>])
#
#   add_goby_library(TARGET <name>
#     [SOURCES <files>...]
#     [PROTOS <proto_files>...]
#     [LINK_LIBRARIES <libs>...]
#     [PROTO_IMPORT_DIRS <dirs>...]
#     [PROTOC_OUT_DIR <dir>]
#     [STATIC] [MODULE])
#
# For add_goby_executable, linking against goby is implied.
# Proto files are compiled with protoc --cpp_out + --dccl_out (matching the
# original protobuf_generate_cpp_dccl behaviour).

# Internal helper: compile .proto files with --cpp_out + --dccl_out.
# Sets OUT_VAR in the caller's scope to the list of generated source files.
function(_goby_generate_protos OUT_VAR PROTOC_OUT_DIR PROTOS IMPORT_DIRS)
  # Build deduplicated list of -I flags
  set(_import_flags)
  set(_seen_dirs)
  foreach(_dir
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}
      ${PROTOC_OUT_DIR}
      ${IMPORT_DIRS}
      ${GOBY_PROTOBUF_IMPORT_DIRS})
    if(_dir)
      get_filename_component(_abs_dir "${_dir}" ABSOLUTE)
      if(NOT "${_abs_dir}" IN_LIST _seen_dirs)
        list(APPEND _seen_dirs "${_abs_dir}")
        list(APPEND _import_flags -I "${_abs_dir}")
      endif()
    endif()
  endforeach()

  set(_all_generated)
  foreach(_proto ${PROTOS})
    get_filename_component(_abs_proto "${_proto}" ABSOLUTE)
    get_filename_component(_proto_we  "${_abs_proto}" NAME_WE)

    set(_pb_h  "${PROTOC_OUT_DIR}/${_proto_we}.pb.h")
    set(_pb_cc "${PROTOC_OUT_DIR}/${_proto_we}.pb.cc")

    # Run protoc with --cpp_out first, then --dccl_out last.
    # This ordering matches the original protobuf_generate_cpp_dccl and ensures
    # that the DCCL plugin can insert into the cpp-generated files.
    add_custom_command(
      OUTPUT "${_pb_h}" "${_pb_cc}"
      COMMAND protobuf::protoc
      ARGS --cpp_out "${PROTOC_OUT_DIR}"
           "${_abs_proto}"
           ${_import_flags}
           --dccl_out "${PROTOC_OUT_DIR}"
      DEPENDS "${_abs_proto}" protobuf::protoc
      COMMENT "Running dccl protocol buffer compiler on ${_proto}"
      VERBATIM)

    set_source_files_properties("${_pb_h}" "${_pb_cc}" PROPERTIES GENERATED TRUE)
    list(APPEND _all_generated "${_pb_h}" "${_pb_cc}")
  endforeach()

  set(${OUT_VAR} "${_all_generated}" PARENT_SCOPE)
endfunction()

# add_goby_executable - build a Goby application binary
# Linking against goby is implied; list any additional libs in LINK_LIBRARIES.
function(add_goby_executable)
  cmake_parse_arguments(args
    ""
    "TARGET;PROTOC_OUT_DIR"
    "SOURCES;PROTOS;LINK_LIBRARIES;PROTO_IMPORT_DIRS"
    ${ARGN})

  if(NOT args_TARGET)
    message(FATAL_ERROR "add_goby_executable: TARGET is required")
  endif()

  set(_sources ${args_SOURCES})

  if(args_PROTOS)
    if(args_PROTOC_OUT_DIR)
      set(_protoc_out_dir "${args_PROTOC_OUT_DIR}")
    else()
      set(_protoc_out_dir "${CMAKE_CURRENT_BINARY_DIR}/${args_TARGET}")
    endif()
    file(MAKE_DIRECTORY "${_protoc_out_dir}")

    _goby_generate_protos(
      _proto_generated
      "${_protoc_out_dir}"
      "${args_PROTOS}"
      "${args_PROTO_IMPORT_DIRS}")

    list(APPEND _sources ${_proto_generated})
  endif()

  add_executable(${args_TARGET} ${_sources})

  if(args_PROTOS)
    target_include_directories(${args_TARGET} PRIVATE "${_protoc_out_dir}")
  endif()

  target_link_libraries(${args_TARGET} goby ${args_LINK_LIBRARIES})

  if(export_goby_interfaces)
    generate_interfaces(${args_TARGET})
  endif()
endfunction()

# add_goby_library - build a Goby library target (SHARED by default)
# Pass STATIC or MODULE to change the library type.
function(add_goby_library)
  cmake_parse_arguments(args
    "STATIC;MODULE"
    "TARGET;PROTOC_OUT_DIR"
    "SOURCES;PROTOS;LINK_LIBRARIES;PROTO_IMPORT_DIRS"
    ${ARGN})

  if(NOT args_TARGET)
    message(FATAL_ERROR "add_goby_library: TARGET is required")
  endif()

  set(_lib_type SHARED)
  if(args_STATIC)
    set(_lib_type STATIC)
  elseif(args_MODULE)
    set(_lib_type MODULE)
  endif()

  set(_sources ${args_SOURCES})

  if(args_PROTOS)
    if(args_PROTOC_OUT_DIR)
      set(_protoc_out_dir "${args_PROTOC_OUT_DIR}")
    else()
      set(_protoc_out_dir "${CMAKE_CURRENT_BINARY_DIR}/${args_TARGET}")
    endif()
    file(MAKE_DIRECTORY "${_protoc_out_dir}")

    _goby_generate_protos(
      _proto_generated
      "${_protoc_out_dir}"
      "${args_PROTOS}"
      "${args_PROTO_IMPORT_DIRS}")

    list(APPEND _sources ${_proto_generated})
  endif()

  add_library(${args_TARGET} ${_lib_type} ${_sources})

  if(args_PROTOS)
    target_include_directories(${args_TARGET} PUBLIC "${_protoc_out_dir}")
  endif()

  if(args_LINK_LIBRARIES)
    target_link_libraries(${args_TARGET} ${args_LINK_LIBRARIES})
  endif()
endfunction()

