add_subdirectory(basic_publisher_subscriber)
# declares its interthread traffic in interface.yml, which an older goby_gen_cpp rejects
if(GOBY_PYTHON_HAS_INTERTHREAD_CONTRACT)
  add_subdirectory(basic_multithread)
else()
  message(STATUS "This Goby's Python generator has no interthread contract: skipping the Python multithread example")
endif()
