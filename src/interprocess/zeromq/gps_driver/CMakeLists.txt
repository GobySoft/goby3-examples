goby_add_app(TARGET goby3_example_gps_driver
  SOURCES gps_driver.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby_zeromq goby3_example_messages)

goby_add_app(TARGET goby3_example_gps_controller
  SOURCES gps_controller.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby_zeromq goby3_example_messages)

if(build_moos)
  goby_add_library(TARGET goby3_example_gps_moos_plugin
    SOURCES moos_plugin.cpp
    PROTOS config.proto
    LINK_LIBRARIES goby goby_zeromq goby_moos goby3_example_messages)
endif()
