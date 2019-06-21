protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR}
  config.proto)

add_executable(goby3_example_basic_interprocess_subscriber
  subscriber.cpp
  ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(goby3_example_basic_interprocess_subscriber
  goby
  goby_zeromq
  goby3_example_messages)
