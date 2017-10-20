protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR} config.proto)

add_executable(gps_driver gps_driver.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(gps_driver goby_middleware example_messages)

add_executable(gps_controller gps_controller.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(gps_controller goby_middleware example_messages)

if(build_moos)
  add_library(gps_moos_plugin SHARED moos_plugin.cpp ${PROTO_SRC} ${PROTO_HDRS})
  target_link_libraries(gps_moos_plugin goby_moos goby_middleware example_messages)
endif()
