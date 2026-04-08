add_goby_executable(TARGET goby3_example_json_publisher
  SOURCES publisher.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby_zeromq goby3_example_messages)

add_goby_executable(TARGET goby3_example_json_subscriber
  SOURCES subscriber.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby_zeromq goby3_example_messages)
