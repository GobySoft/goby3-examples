goby_add_app(TARGET goby3_example_trail_usv_manager
  SOURCES usv_manager.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby_zeromq goby3_example_trail_messages)
