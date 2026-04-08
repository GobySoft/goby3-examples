set(protoc_out_dir ${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate(
  LANGUAGE dccl
  PROTOC_OPTIONS --cpp_out=${protoc_out_dir}
  PROTOC_OUT_DIR ${protoc_out_dir}
  PROTOS udp_multicast_driver.proto
  GENERATE_EXTENSIONS .pb.h .pb.cc
  IMPORT_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${protoc_out_dir}
    ${GOBY_PROTOBUF_IMPORT_DIRS}
  OUT_VAR PROTO_GENERATED
)

add_library(goby3_example_pluggable_udp_multicast_driver SHARED udp_multicast_driver.cpp ${PROTO_GENERATED})
target_include_directories(goby3_example_pluggable_udp_multicast_driver PRIVATE ${protoc_out_dir})
target_link_libraries(goby3_example_pluggable_udp_multicast_driver goby)

