add_goby_executable(TARGET dccl_simple
  SOURCES dccl_simple.cpp
  PROTOS simple.proto)
install(TARGETS dccl_simple RUNTIME DESTINATION bin)
