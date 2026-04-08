add_goby_executable(TARGET queue_multimessage
  SOURCES multimessage.cpp
  PROTOS ${CMAKE_CURRENT_SOURCE_DIR}/../../dccl/dccl_simple/simple.proto
  PROTO_IMPORT_DIRS ${CMAKE_SOURCE_DIR}/src/components/acomms/dccl/dccl_simple)
install(TARGETS queue_multimessage RUNTIME DESTINATION bin)
