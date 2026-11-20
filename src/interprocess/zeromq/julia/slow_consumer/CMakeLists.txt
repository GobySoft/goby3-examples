goby_add_library(TARGET goby3_example_julia_slow_consumer_config
  PROTOS slow_consumer_config.proto
  PROTOC_OUT_DIR ${project_INC_DIR}/julia_slow_consumer_config
  LINK_LIBRARIES ${Protobuf_LIBRARIES} dccl)

goby_add_julia_app(TARGET basic_julia_slow_consumer
  INTERFACE_YML interface.yml
  MAIN slow_consumer.jl
  # one per task module plus three, for Main, the loop timer and the C++ application
  THREADS 6
  INCLUDES messages/julia_queue_stats.pb.h messages/groups.h
           julia_slow_consumer_config/slow_consumer_config.pb.h
           ${GOBY_INCLUDE_DIR}/goby/zeromq/application/single_thread.h
  LINK_LIBRARIES goby goby_zeromq goby3_example_messages goby3_example_julia_slow_consumer_config)

target_include_directories(basic_julia_slow_consumer PRIVATE ${project_INC_DIR})

goby_add_julia_protos(PACKAGE goby3_examples PROTOS slow_consumer_config.proto)
