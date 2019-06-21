protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${project_INC_DIR}/messages
  nav.proto
  gps.proto
  state.proto
  )

add_library(goby3_example_messages SHARED ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(goby3_example_messages ${PROTOBUF_LIBRARIES} dccl)
