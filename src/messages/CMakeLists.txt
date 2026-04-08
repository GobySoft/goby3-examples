set(protoc_out_dir ${project_INC_DIR}/messages)
file(MAKE_DIRECTORY ${protoc_out_dir})

protobuf_generate(
  LANGUAGE dccl
  PROTOC_OPTIONS --cpp_out=${protoc_out_dir}
  PROTOC_OUT_DIR ${protoc_out_dir}
  PROTOS nav.proto nav_dccl.proto gps.proto state.proto init.proto
  GENERATE_EXTENSIONS .pb.h .pb.cc
  IMPORT_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${protoc_out_dir}
    ${GOBY_PROTOBUF_IMPORT_DIRS}
  OUT_VAR PROTO_GENERATED
)

add_library(goby3_example_messages SHARED ${PROTO_GENERATED})
target_include_directories(goby3_example_messages PUBLIC ${protoc_out_dir})
target_link_libraries(goby3_example_messages ${Protobuf_LIBRARIES} dccl)

