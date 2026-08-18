goby_add_app(TARGET goby3_example_basic_intervehicle_publisher
  SOURCES publisher.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby_zeromq goby3_example_messages)
