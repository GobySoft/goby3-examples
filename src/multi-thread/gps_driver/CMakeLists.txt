protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR} config.proto)

add_executable(gps_driver gps_driver.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(gps_driver goby goby_zeromq example_messages)

add_executable(gps_controller gps_controller.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(gps_controller goby goby_zeromq example_messages)

