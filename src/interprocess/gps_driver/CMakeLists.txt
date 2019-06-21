protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR} config.proto)

add_executable(goby3_example_gps_driver gps_driver.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(goby3_example_gps_driver
  goby goby_zeromq goby3_example_messages)

add_executable(goby3_example_gps_controller gps_controller.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(goby3_example_gps_controller
  goby goby_zeromq goby3_example_messages)

