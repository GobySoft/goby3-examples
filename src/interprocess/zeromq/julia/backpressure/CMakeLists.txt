goby_add_library(TARGET goby3_example_julia_backpressure_config
  PROTOS backpressure_config.proto
  PROTOC_OUT_DIR ${project_INC_DIR}/julia_backpressure_config
  LINK_LIBRARIES ${Protobuf_LIBRARIES} dccl)

goby_add_julia_app(TARGET basic_julia_backpressure
  INTERFACE_YML interface.yml
  MAIN backpressure.jl
  # one per task module plus three, for Main, the loop timer and the C++ application
  THREADS 6
  INCLUDES messages/julia_backpressure.pb.h messages/groups.h
           julia_backpressure_config/backpressure_config.pb.h
           ${GOBY_INCLUDE_DIR}/goby/zeromq/application/single_thread.h
  LINK_LIBRARIES goby goby_zeromq goby3_example_messages goby3_example_julia_backpressure_config)

target_include_directories(basic_julia_backpressure PRIVATE ${project_INC_DIR})

goby_add_julia_protos(PACKAGE goby3_examples PROTOS backpressure_config.proto)
