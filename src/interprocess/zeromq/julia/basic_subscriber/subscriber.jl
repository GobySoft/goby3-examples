# Julia subscriber equivalent to basic_subscriber.
#
# Subscribes to NavigationReport messages on the "navigation" group and
# prints each received message.
#
# Run with:
#   julia build/julia/basic_subscriber/subscriber.jl <config.pb.cfg>

const GOBY_APPLICATION_LIB = joinpath(@__DIR__, "libbasic_julia_subscriber")
include(joinpath(@__DIR__, "..", "goby_application_init.jl"))

# ---------------------------------------------------------------------------
# start() is called by Goby.run() before entering the event loop.
# ---------------------------------------------------------------------------
function start()
    Goby.subscribe(Main.app, Goby.INTERPROCESS, "navigation",
        (nav::NavigationReport) -> println("Rx: x=$(nav.x) y=$(nav.y) z=$(nav.z)")
    )
end

config_str = Goby.read_cli_cfg()
app = BasicJuliaSubscriber(config_str)
Goby.run(app)
