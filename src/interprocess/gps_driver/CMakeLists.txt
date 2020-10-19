protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR} config.proto)

add_executable(goby3_example_gps_driver gps_driver.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(goby3_example_gps_driver
  goby goby_zeromq goby3_example_messages)

add_executable(goby3_example_gps_controller gps_controller.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(goby3_example_gps_controller
  goby goby_zeromq goby3_example_messages)

if(build_moos)
  add_library(gps_moos_plugin SHARED moos_plugin.cpp ${PROTO_SRC} ${PROTO_HDRS})
  target_link_libraries(gps_moos_plugin goby goby_zeromq goby_moos goby3_example_messages)
endif()

if(export_goby_interfaces)
  generate_interfaces(goby3_example_gps_driver)
  generate_interfaces(goby3_example_gps_controller)
endif()
