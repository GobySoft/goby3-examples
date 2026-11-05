goby_add_library(TARGET goby3_example_trail_messages
  PROTOS nav.proto
  PROTOC_OUT_DIR ${project_INC_DIR}/trail
  LINK_LIBRARIES ${Protobuf_LIBRARIES} dccl)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(usv_manager)
add_subdirectory(auv_manager)
add_subdirectory(topside_manager)

if(build_moos)
  add_subdirectory(moos_plugin)
else()
  message(">> the intervehicle trail example needs -Dbuild_moos=ON for the AUVs to trail the USV")
endif()
