goby_add_app(TARGET goby3_example_basic_intervehicle_subscriber
  SOURCES subscriber.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby_zeromq goby3_example_messages)
