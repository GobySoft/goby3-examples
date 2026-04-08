set(protoc_out_dir ${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate(
  LANGUAGE dccl
  PROTOC_OPTIONS --cpp_out=${protoc_out_dir}
  PROTOC_OUT_DIR ${protoc_out_dir}
  PROTOS simple_status.proto
  GENERATE_EXTENSIONS .pb.h .pb.cc
  IMPORT_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${protoc_out_dir}
    ${GOBY_PROTOBUF_IMPORT_DIRS}
  OUT_VAR PROTO_GENERATED
)

add_library(gobyexample_protobuf SHARED ${PROTO_GENERATED})
target_include_directories(gobyexample_protobuf PUBLIC ${protoc_out_dir})
target_link_libraries(gobyexample_protobuf goby)

