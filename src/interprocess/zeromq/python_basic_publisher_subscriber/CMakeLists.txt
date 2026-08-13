add_goby_python_app(TARGET goby3_example_python_basic_publisher
  INTERFACE_YML publisher.yml
  MAIN publisher.py
  # compiled for C++ (the extension module needs the configuration type) and for Python
  PROTOS config.proto
  # C++ side is in goby3_example_messages; only the Python module is missing
  PYTHON_PROTOS ${project_INC_DIR}/messages/nav.proto
  PYTHON_PROTO_MODULES config_pb2 messages.nav_pb2
  # headers the generated C++ glue needs for the groups and types named in publisher.yml
  INCLUDES goby/zeromq/application/single_thread.h
           messages/groups.h messages/nav.pb.h config.pb.h
  LINK_LIBRARIES goby_zeromq goby3_example_messages)

add_goby_python_app(TARGET goby3_example_python_basic_subscriber
  INTERFACE_YML subscriber.yml
  MAIN subscriber.py
  PROTOS config.proto
  PYTHON_PROTOS ${project_INC_DIR}/messages/nav.proto
  PYTHON_PROTO_MODULES config_pb2 messages.nav_pb2
  INCLUDES goby/zeromq/application/single_thread.h
           messages/groups.h messages/nav.pb.h config.pb.h
  LINK_LIBRARIES goby_zeromq goby3_example_messages)
