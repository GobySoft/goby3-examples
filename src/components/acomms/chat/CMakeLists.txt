set(CURSES_NEED_NCURSES TRUE)
find_package(Curses)

if(CURSES_FOUND)
  add_goby_executable(TARGET goby3_example_chat
    SOURCES chat.cpp chat_curses.cpp
    PROTOS chat.proto
    LINK_LIBRARIES ${CURSES_LIBRARIES})
else()
  message(WARNING "Missing NCurses so not building `chat` example")
endif()
