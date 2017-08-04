protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR} config.proto)

add_executable(basic_multithread_pubsub main.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(basic_multithread_pubsub goby_middleware example_messages)

