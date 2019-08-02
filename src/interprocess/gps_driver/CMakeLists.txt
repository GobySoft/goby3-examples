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
  set(YML_OUT_DIR ${project_SHARE_DIR}/interfaces)
  goby_export_interface(goby3_example_gps_driver ${YML_OUT_DIR} DRIVER_YML_OUT)
  goby_export_interface(goby3_example_gps_controller ${YML_OUT_DIR} CONTROLLER_YML_OUT)

  set(PDF_OUT ${YML_OUT_DIR}/dot/gps_interfaces.pdf)
  goby_visualize_interfaces(${YML_OUT_DIR} gps_deployment.yml ${PDF_OUT}
    ${DRIVER_YML_OUT}
    ${CONTROLLER_YML_OUT})

  add_custom_command(TARGET gps_deployment_interface_viz POST_BUILD COMMAND evince ARGS ${PDF_OUT}&)
endif()
