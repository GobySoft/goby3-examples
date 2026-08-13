# Julia subscriber equivalent to basic_subscriber.
#
# Subscribes to NavigationReport messages on the "navigation" group and
# prints each received message.
#
# Run with:
#   julia build/julia/basic_subscriber/subscriber.jl <config.pb.cfg>

include(joinpath(@__DIR__, "..", "goby_application_init.jl"))
init_cxxwrap("basic_julia_subscriber")
Test.@include_protos()

# ---------------------------------------------------------------------------
# start() is called by Goby.run() before entering the event loop.
# ---------------------------------------------------------------------------
function receive_incoming_msg(nav::goby3_examples.protobuf.NavigationReport)
    println("Rx: x=$(nav.x) y=$(nav.y) z=$(nav.z)")
end

function start()
    Goby.subscribe(Main.app, Goby.INTERPROCESS, "groups::nav2", receive_incoming_msg)
end

config_str = Goby.read_cli_cfg()
app = Goby.BasicJuliaSubscriber(config_str)
Goby.run(app)
