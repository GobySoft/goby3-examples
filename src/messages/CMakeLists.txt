add_goby_library(TARGET goby3_example_messages
  PROTOS nav.proto nav_dccl.proto gps.proto state.proto init.proto julia_nav.proto
  PROTOC_OUT_DIR ${project_INC_DIR}/messages
  LINK_LIBRARIES ${Protobuf_LIBRARIES} dccl)

if(enable_julia_examples)
  goby_add_julia_protos(PACKAGE goby3_examples PROTOS julia_nav.proto)
endif()
