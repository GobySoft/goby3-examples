add_subdirectory(zeromq)

if(EXISTS "${GOBY_INCLUDE_DIR}/goby/udpm/protobuf/interprocess_config.proto")
    add_subdirectory(udpm)
else()
    message(WARNING "Unable to build udpm interprocess examples. UDPM headers not found (requires goby >= 3.3.1+49).")
endif()
