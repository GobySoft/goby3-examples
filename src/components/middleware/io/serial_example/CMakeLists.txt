add_goby_executable(TARGET goby3_example_serial
  SOURCES main.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby_zeromq goby3_example_messages)
