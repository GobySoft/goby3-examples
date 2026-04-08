add_goby_executable(TARGET goby3_example_intervehicle_same_group
  SOURCES app.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby_zeromq goby3_example_messages)
