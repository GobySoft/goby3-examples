goby_add_app(TARGET goby3_example_trail_topside_manager
  SOURCES topside_manager.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby_zeromq goby3_example_trail_messages)
