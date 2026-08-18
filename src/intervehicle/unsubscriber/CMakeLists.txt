goby_add_app(TARGET goby3_example_intervehicle_unsubscriber
  SOURCES unsubscriber.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby_zeromq goby3_example_messages)

