add_goby_executable(TARGET whoi_ranging
  SOURCES whoi_ranging.cpp)
install(TARGETS whoi_ranging RUNTIME DESTINATION bin)
