set(JULIA_OUT_DIR "${project_BUILD_DIR}/julia")

# Copy shared Julia initialization code to the build directory so it can be
# included by both the publisher and subscriber scripts at runtime.
configure_file(goby_application_init.jl "${JULIA_OUT_DIR}/goby_application_init.jl" COPYONLY)

add_subdirectory(basic_publisher)
add_subdirectory(basic_subscriber)

# these use INTERTHREAD's per-subscription queues, and basic_multithread declares its interthread
# traffic in interface.yml, which older Goby.jl rejects
if(GOBY_JULIA_HAS_INTERTHREAD_CONTRACT)
  add_subdirectory(basic_multithread)
  add_subdirectory(backpressure)
else()
  message(STATUS "This Goby.jl has no INTERTHREAD queues or contract: skipping the Julia multithread and backpressure examples")
endif()
