# both applications share the configuration messages
goby_add_library(TARGET goby3_example_python_config
  PROTOS config.proto
  PROTOC_OUT_DIR ${project_INC_DIR}/python_config
  LINK_LIBRARIES ${Protobuf_LIBRARIES} dccl)

goby_add_python_app(TARGET goby3_example_python_basic_publisher
  INTERFACE_YML publisher.yml
  MAIN publisher.py
  PYTHON_PROTOS config.proto ${project_INC_DIR}/messages/nav.proto
  PROTO_IMPORT_DIRS ${project_INC_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
  PROTO_MODULES config_pb2 messages.nav_pb2
  # headers for the groups and types named in publisher.yml
  INCLUDES goby/zeromq/application/single_thread.h
           messages/groups.h messages/nav.pb.h python_config/config.pb.h
  LINK_LIBRARIES goby_zeromq goby3_example_messages goby3_example_python_config)

goby_add_python_app(TARGET goby3_example_python_basic_subscriber
  INTERFACE_YML subscriber.yml
  MAIN subscriber.py
  PYTHON_PROTOS config.proto ${project_INC_DIR}/messages/nav.proto
  PROTO_IMPORT_DIRS ${project_INC_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
  PROTO_MODULES config_pb2 messages.nav_pb2
  INCLUDES goby/zeromq/application/single_thread.h
           messages/groups.h messages/nav.pb.h python_config/config.pb.h
  LINK_LIBRARIES goby_zeromq goby3_example_messages goby3_example_python_config)
