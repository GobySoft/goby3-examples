protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR} config.proto)

add_executable(state_machine_subscriber subscriber.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(state_machine_subscriber goby_middleware example_messages)

add_executable(state_machine_publisher publisher.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(state_machine_publisher goby_middleware example_messages)
