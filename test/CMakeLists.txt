# ctest wrappers around the goby pub/sub test harnesses in this directory.
# Each test launches the example, runs 'goby <transport> test', and shuts the example down.

set(run ${CMAKE_CURRENT_SOURCE_DIR}/run_test.sh)
set(launch ${CMAKE_SOURCE_DIR}/launch)
set(tests ${CMAKE_CURRENT_SOURCE_DIR})

add_test(NAME goby3_example_test_basic_interprocess
  COMMAND ${run} ${tests}/interprocess/zeromq/basic_publisher_subscriber.test.yml
                 ${launch}/interprocess/zeromq/basic_publisher_subscriber.launch)
add_test(NAME goby3_example_test_state_machine
  COMMAND ${run} ${tests}/interprocess/zeromq/state_machine.test.yml
                 ${launch}/interprocess/zeromq/state_machine_app_only.launch)
add_test(NAME goby3_example_test_json
  COMMAND ${run} ${tests}/interprocess/zeromq/json.test.yml
                 ${launch}/interprocess/zeromq/json.launch)
add_test(NAME goby3_example_test_basic_udpm
  COMMAND ${run} ${tests}/interprocess/udpm/basic_publisher_subscriber.test.yml
                 ${launch}/interprocess/udpm/basic_publisher_subscriber.launch)
add_test(NAME goby3_example_test_basic_intervehicle
  COMMAND ${run} ${tests}/intervehicle/basic.test.yml
                 ${launch}/intervehicle/basic/basic_vehicle1.launch
                 ${launch}/intervehicle/basic/basic_vehicle2.launch)

set_tests_properties(goby3_example_test_basic_intervehicle PROPERTIES TIMEOUT 120)
foreach(t basic_interprocess state_machine json basic_udpm)
  set_tests_properties(goby3_example_test_${t} PROPERTIES TIMEOUT 60)
endforeach()
# the harnesses share platforms and multicast groups, so never run them concurrently
set_tests_properties(goby3_example_test_basic_interprocess goby3_example_test_state_machine
  goby3_example_test_json goby3_example_test_basic_udpm goby3_example_test_basic_intervehicle
  PROPERTIES RUN_SERIAL TRUE)
