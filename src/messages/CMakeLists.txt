protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${project_INC_DIR}/messages nav.proto)

add_library(example_messages SHARED ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(example_messages ${PROTOBUF_LIBRARIES} dccl)
