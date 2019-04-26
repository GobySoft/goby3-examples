protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR} config.proto)

add_executable(multithread_configurator main.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(multithread_configurator goby_middleware example_messages)

