goby_add_app(TARGET goby3_example_basic_zenoh_interprocess_subscriber
  SOURCES subscriber.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby3_example_messages goby_zenoh)
