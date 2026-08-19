# Julia subscriber equivalent to basic_subscriber.
#
# Subscribes to NavigationReport messages on the "groups::julia_nav" group and
# prints each received message.
#
# Run with:
#   julia build/julia/basic_julia_subscriber/subscriber.jl <config.pb.cfg>

include(joinpath(@__DIR__, "..", "goby_application_init.jl"))
init_cxxwrap("basic_julia_subscriber")
include_protos(@__MODULE__, "goby3_examples")

# the groups and layer accessors generated from interface.yml, written beside this script by
# goby_add_julia_app()
include(joinpath(@__DIR__, "basic_julia_subscriber_goby.jl"))
using .BasicJuliaSubscriberGoby: groups, interprocess

# ---------------------------------------------------------------------------
# Called by Goby for each NavigationReport received. The message type is
# inferred by Goby.subscribe() from this argument type.
# ---------------------------------------------------------------------------
function receive_incoming_msg(nav::goby3_examples.protobuf.NavigationReport)
    println("Rx: x=$(nav.x) y=$(nav.y) z=$(nav.z)")
end

# start() is called by Goby.run() before entering the event loop.
function start()
    Goby.subscribe(Main.app, interprocess(), groups.julia_nav, receive_incoming_msg)
end

config_str = Goby.read_cli_cfg()
app = Goby.BasicJuliaSubscriber(config_str)
Goby.run(app)
