# Each harness launches its example as a fixture, so ctest runs 'goby test' directly.
# The examples must be on the PATH for goby_launch, along with their message library.

execute_process(COMMAND goby test help OUTPUT_VARIABLE goby_test_help ERROR_VARIABLE goby_test_help
  RESULT_VARIABLE goby_test_result)
if(NOT goby_test_help MATCHES "validate")
  message(FATAL_ERROR "enable_tests requires a goby that provides 'goby test run' and 'goby test validate'")
endif()

set(test_env
  "PATH=${project_BIN_DIR}:$ENV{PATH}"
  "GOBY_TOOL_LOAD_SHARED_LIBRARY=${project_LIB_DIR}/libgoby3_example_messages.so")

set(harnesses
  basic_interprocess:interprocess/zeromq/basic_publisher_subscriber.test.yml
  state_machine:interprocess/zeromq/state_machine.test.yml
  json:interprocess/zeromq/json.test.yml
  basic_udpm:interprocess/udpm/basic_publisher_subscriber.test.yml
  basic_intervehicle:intervehicle/basic.test.yml)

foreach(harness ${harnesses})
  string(REPLACE ":" ";" parts ${harness})
  list(GET parts 0 name)
  list(GET parts 1 file)
  set(path ${CMAKE_CURRENT_SOURCE_DIR}/${file})

  add_test(NAME goby3_example_validate_${name} COMMAND goby test validate ${path})
  add_test(NAME goby3_example_test_${name} COMMAND goby test run ${path})
  set_tests_properties(goby3_example_validate_${name} goby3_example_test_${name}
    PROPERTIES ENVIRONMENT "${test_env}")
  # the harnesses share platforms and multicast groups, so never run them concurrently
  set_tests_properties(goby3_example_test_${name} PROPERTIES RUN_SERIAL TRUE TIMEOUT 120)
endforeach()
