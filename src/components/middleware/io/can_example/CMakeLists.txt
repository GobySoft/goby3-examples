protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR}
  config.proto)

set (APP_NAME goby3_example_can)

add_executable( ${APP_NAME}
  main.cpp
  ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries( ${APP_NAME}
  goby
  goby_zeromq
  goby3_example_messages)

if(export_goby_interfaces)
  generate_interfaces(${APP_NAME})
endif()
