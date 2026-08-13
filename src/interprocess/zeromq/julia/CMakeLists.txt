set(JULIA_OUT_DIR "${project_BUILD_DIR}/julia")

# Copy shared Julia initialization code to the build directory so it can be
# included by both the publisher and subscriber scripts at runtime.
configure_file(goby_application_init.jl "${JULIA_OUT_DIR}/goby_application_init.jl" COPYONLY)

# Generate Julia protobuf bindings for nav.proto using ProtoBuf.jl's built-in
# code generator (ProtoBuf.protojl).  The output is placed at
#   ${JULIA_OUT_DIR}/protobuf/protobuf.jl
# and shared between the publisher and subscriber scripts.
#goby_generate_julia_proto(
#    "${CMAKE_SOURCE_DIR}/src/messages/nav.proto"
#    "${CMAKE_SOURCE_DIR}/src/"
#    "${JULIA_OUT_DIR}"
#)

add_subdirectory(basic_publisher)
add_subdirectory(basic_subscriber)
