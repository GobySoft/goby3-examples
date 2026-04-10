set(JULIA_OUT_DIR "${project_BUILD_DIR}/julia")

# Generate Julia protobuf bindings for nav.proto using ProtoBuf.jl's built-in
# code generator (ProtoBuf.protojl).  The output is placed at
#   ${JULIA_OUT_DIR}/nav_pb.jl
# and shared between the publisher and subscriber scripts.
goby_generate_julia_proto(
    "${CMAKE_SOURCE_DIR}/src/messages/nav.proto"
    "${CMAKE_SOURCE_DIR}/src/messages"
    "${JULIA_OUT_DIR}"
)

add_subdirectory(basic_publisher)
add_subdirectory(basic_subscriber)
