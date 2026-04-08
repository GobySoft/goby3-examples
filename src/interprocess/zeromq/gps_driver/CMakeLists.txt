add_goby_executable(TARGET goby3_example_gps_driver
  SOURCES gps_driver.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby_zeromq goby3_example_messages)

add_goby_executable(TARGET goby3_example_gps_controller
  SOURCES gps_controller.cpp
  PROTOS config.proto
  LINK_LIBRARIES goby_zeromq goby3_example_messages)

if(build_moos)
  set(protoc_out_dir ${CMAKE_CURRENT_BINARY_DIR}/gps_moos_plugin)
  file(MAKE_DIRECTORY ${protoc_out_dir})
  protobuf_generate(
    LANGUAGE dccl
    PROTOC_OPTIONS --cpp_out=${protoc_out_dir}
    PROTOC_OUT_DIR ${protoc_out_dir}
    PROTOS config.proto
    GENERATE_EXTENSIONS .pb.h .pb.cc
    IMPORT_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${protoc_out_dir}
      ${GOBY_PROTOBUF_IMPORT_DIRS}
    OUT_VAR MOOS_PROTO_GENERATED
  )
  add_library(gps_moos_plugin SHARED moos_plugin.cpp ${MOOS_PROTO_GENERATED})
  target_include_directories(gps_moos_plugin PRIVATE ${protoc_out_dir})
  target_link_libraries(gps_moos_plugin goby goby_zeromq goby_moos goby3_example_messages)
endif()
