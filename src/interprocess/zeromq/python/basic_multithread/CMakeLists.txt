goby_add_library(TARGET goby3_example_python_multithread_config
  PROTOS multithread_config.proto
  PROTOC_OUT_DIR ${project_INC_DIR}/python_multithread_config
  LINK_LIBRARIES ${Protobuf_LIBRARIES} dccl)

goby_add_python_app(TARGET goby3_example_python_multithread
  INTERFACE_YML interface.yml
  MAIN multithread.py
  PYTHON_PROTOS multithread_config.proto ${project_INC_DIR}/messages/nav.proto
  PROTO_IMPORT_DIRS ${project_INC_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
  PROTO_MODULES multithread_config_pb2 messages.nav_pb2
  # headers for the group and type named in interface.yml; the interthread groups need none
  INCLUDES goby/zeromq/application/single_thread.h
           messages/groups.h messages/nav.pb.h
           python_multithread_config/multithread_config.pb.h
  LINK_LIBRARIES goby_zeromq goby3_example_messages goby3_example_python_multithread_config)
