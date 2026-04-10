add_goby_library(TARGET goby3_example_messages
  PROTOS nav.proto nav_dccl.proto gps.proto state.proto init.proto julia_nav.proto
  PROTOC_OUT_DIR ${project_INC_DIR}/messages
  LINK_LIBRARIES ${Protobuf_LIBRARIES} dccl)


protobuf_generate_julia("goby3_examples" "julia_nav.proto")
