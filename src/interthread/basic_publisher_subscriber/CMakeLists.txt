protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR}
  config.proto)

add_executable(goby3_example_basic_interthread
  main.cpp
  ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(goby3_example_basic_interthread
  goby
  goby_zeromq
  goby3_example_messages)


if(export_goby_interfaces)
  goby_export_interface(goby3_example_basic_interthread ${project_SHARE_DIR}/interfaces YML_OUT)
  add_custom_target(goby3_example_basic_interthread_interface ALL DEPENDS ${YML_OUT})
endif()
