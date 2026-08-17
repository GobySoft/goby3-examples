add_goby_library(TARGET goby3_example_julia_publisher_config
  PROTOS publisher_config.proto
  PROTOC_OUT_DIR ${project_INC_DIR}/julia_publisher_config
  LINK_LIBRARIES ${Protobuf_LIBRARIES} dccl)

goby_add_julia_app(TARGET basic_julia_publisher
  INTERFACE_YML interface.yml
  MAIN publisher.jl
  INCLUDES messages/julia_nav.pb.h messages/groups.h
           julia_publisher_config/publisher_config.pb.h
           ${GOBY_INCLUDE_DIR}/goby/zeromq/application/single_thread.h
  LINK_LIBRARIES goby goby_zeromq goby3_example_messages goby3_example_julia_publisher_config)

target_include_directories(basic_julia_publisher PRIVATE ${project_INC_DIR})

goby_add_julia_protos(PACKAGE goby3_examples PROTOS publisher_config.proto)
