add_goby_julia_application(
    TARGET basic_julia_publisher
    SCRIPT publisher.jl
    INTERFACE_YML interface.yml
    CONFIG_PROTO config.proto
    LINK_LIBRARIES goby3_example_messages
    INCLUDE_HEADERS messages/nav.pb.h messages/groups.h
    JULIA_PROTO_TARGETS julia_proto_nav
)

