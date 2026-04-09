add_goby_executable(TARGET goby3_example_queue_simple
  SOURCES queue_simple.cpp
  PROTOS ${CMAKE_CURRENT_SOURCE_DIR}/../../dccl/dccl_simple/simple.proto
  PROTO_IMPORT_DIRS ${CMAKE_SOURCE_DIR}/src/components/acomms/dccl/dccl_simple)
