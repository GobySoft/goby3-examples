add_goby_executable(TARGET goby3_example_gps_driver
  SOURCES gps_driver.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby_zeromq goby3_example_messages)

add_goby_executable(TARGET goby3_example_gps_controller
  SOURCES gps_controller.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby_zeromq goby3_example_messages)

if(build_moos)
  add_goby_library(TARGET gps_moos_plugin
    SOURCES moos_plugin.cpp
    PROTOS config.proto
    LINK_LIBRARIES goby goby_zeromq goby_moos goby3_example_messages)
endif()
