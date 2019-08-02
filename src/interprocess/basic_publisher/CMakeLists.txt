protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR}
  config.proto)

add_executable(goby3_example_basic_interprocess_publisher
  publisher.cpp
  ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(goby3_example_basic_interprocess_publisher
  goby
  goby_zeromq
  goby3_example_messages)

if(export_goby_interfaces)
  goby_export_interface(goby3_example_basic_interprocess_publisher ${project_SHARE_DIR}/interfaces YML_OUT)
  add_custom_target(goby3_example_basic_interprocess_publisher_interface ALL DEPENDS ${YML_OUT})
endif()
