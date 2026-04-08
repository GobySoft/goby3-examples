add_goby_executable(TARGET two_message
  SOURCES two_message.cpp
  PROTOS two_message.proto)
install(TARGETS two_message RUNTIME DESTINATION bin)
