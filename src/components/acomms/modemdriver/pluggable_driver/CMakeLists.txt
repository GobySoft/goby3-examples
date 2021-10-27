protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR} udp_multicast_driver.proto)

add_library(goby3_example_pluggable_udp_multicast_driver SHARED udp_multicast_driver.cpp ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(goby3_example_pluggable_udp_multicast_driver goby)
