# add_goby_executable - convenience function for building Goby application binaries
#
# Usage:
#   add_goby_executable(TARGET <target_name>
#     SOURCES <source_files>...
#     [PROTOS <proto_files>...]
#     [LINK_LIBRARIES <libs>...]
#     [PROTO_IMPORT_DIRS <dirs>...]
#   )
#
# The function:
#   - Compiles any .proto files using protobuf_generate (DCCL language)
#   - Creates the executable via add_executable
#   - Links against goby (implied) plus any LINK_LIBRARIES specified
#   - Calls generate_interfaces() when export_goby_interfaces is ON

function(add_goby_executable)
  cmake_parse_arguments(args "" "TARGET" "SOURCES;PROTOS;LINK_LIBRARIES;PROTO_IMPORT_DIRS" ${ARGN})

  if(NOT args_TARGET)
    message(FATAL_ERROR "add_goby_executable: TARGET is required")
  endif()

  if(args_PROTOS)
    set(protoc_out_dir ${CMAKE_CURRENT_BINARY_DIR}/${args_TARGET})
    file(MAKE_DIRECTORY ${protoc_out_dir})

    add_executable(${args_TARGET} ${args_SOURCES})

    protobuf_generate(
      LANGUAGE dccl
      PROTOC_OPTIONS --cpp_out=${protoc_out_dir}
      PROTOC_OUT_DIR ${protoc_out_dir}
      PROTOS ${args_PROTOS}
      GENERATE_EXTENSIONS .pb.h .pb.cc
      IMPORT_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${args_PROTO_IMPORT_DIRS}
        ${GOBY_PROTOBUF_IMPORT_DIRS}
      TARGET ${args_TARGET}
    )

    target_include_directories(${args_TARGET} PRIVATE ${protoc_out_dir})
  else()
    add_executable(${args_TARGET} ${args_SOURCES})
  endif()

  target_link_libraries(${args_TARGET}
    goby
    ${args_LINK_LIBRARIES})

  if(export_goby_interfaces)
    generate_interfaces(${args_TARGET})
  endif()
endfunction()
