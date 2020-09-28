protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR}
  config.proto)

set (APP_NAME goby3_example_tcp)

add_executable(${APP_NAME}
  main.cpp
  ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(${APP_NAME}
  goby
  goby_zeromq
  goby3_example_messages)

#if(export_goby_interfaces)
#  goby_export_interface(${APP_NAME} ${project_SHARE_DIR}/interfaces YML_OUT)
#  add_custom_target(${APP_NAME}_interface ALL DEPENDS ${YML_OUT})
#endif()
