protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR}
  config.proto)

add_executable(goby3_example_json_publisher
  publisher.cpp
  ${PROTO_SRCS} ${PROTO_HDRS})

add_executable(goby3_example_json_subscriber
  subscriber.cpp
  ${PROTO_SRCS} ${PROTO_HDRS})


target_link_libraries(goby3_example_json_publisher
  goby
  goby_zeromq
  goby3_example_messages)

target_link_libraries(goby3_example_json_subscriber
  goby
  goby_zeromq
  goby3_example_messages)


if(export_goby_interfaces)
  generate_interfaces(goby3_example_json_publisher)
  generate_interfaces(goby3_example_json_subscriber)
endif()
