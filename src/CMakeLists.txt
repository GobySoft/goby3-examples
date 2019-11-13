option(build_moos "Build MOOS related examples" OFF)

if(build_moos)
  find_package(GOBY 3.0 REQUIRED moos)
endif()

add_subdirectory(messages)

add_subdirectory(interthread)
add_subdirectory(interprocess)
add_subdirectory(intervehicle)

add_subdirectory(components)

