goby_add_app(TARGET goby3_example_basic_udpm_interprocess_publisher
  SOURCES publisher.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby3_example_messages)
