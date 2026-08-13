set(JULIA_OUT_DIR "${project_BUILD_DIR}/julia")

# Copy shared Julia initialization code to the build directory so it can be
# included by both the publisher and subscriber scripts at runtime.
configure_file(goby_application_init.jl "${JULIA_OUT_DIR}/goby_application_init.jl" COPYONLY)

add_subdirectory(basic_publisher)
add_subdirectory(basic_subscriber)
