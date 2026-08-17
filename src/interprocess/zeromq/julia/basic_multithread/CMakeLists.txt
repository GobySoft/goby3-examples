add_goby_library(TARGET goby3_example_julia_multithread_config
  PROTOS config.proto
  PROTOC_OUT_DIR ${project_INC_DIR}/julia_multithread_config
  LINK_LIBRARIES ${Protobuf_LIBRARIES} dccl)

goby_add_julia_app(TARGET basic_julia_multithread
  INTERFACE_YML interface.yml
  MAIN multithread.jl
  # one per task module plus three, for Main, the loop timer and the C++ application
  THREADS 5
  INCLUDES messages/julia_nav.pb.h messages/groups.h
           julia_multithread_config/config.pb.h
           ${GOBY_INCLUDE_DIR}/goby/zeromq/application/single_thread.h
  LINK_LIBRARIES goby goby_zeromq goby3_example_messages goby3_example_julia_multithread_config)

target_include_directories(basic_julia_multithread PRIVATE ${project_INC_DIR})
