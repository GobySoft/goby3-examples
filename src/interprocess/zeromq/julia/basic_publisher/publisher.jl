# Julia publisher equivalent to basic_publisher.
#
# Publishes NavigationReport messages at 10 Hz on the "navigation" group.
#
# Run with:
#   julia build/julia/basic_publisher/publisher.jl <config.pb.cfg>

const GOBY_APPLICATION_LIB = joinpath(@__DIR__, "libbasic_julia_publisher")
include(joinpath(@__DIR__, "..", "goby_application_init.jl"))

# ---------------------------------------------------------------------------
# Publisher loop – called at the frequency specified in goby_cfg below.
# ---------------------------------------------------------------------------
function loop()
    nav = NavigationReport(
        x = 95.0 + rand() * 20,
        y = 195.0 + rand() * 20,
        z = -305.0 + rand() * 10,
    )
    println("Tx: x=$(nav.x) y=$(nav.y) z=$(nav.z)")
    Goby.publish(Main.app, Goby.INTERPROCESS, "navigation", nav)
end

# goby_cfg is inspected by Goby.run() to configure the loop frequency.
goby_cfg = Dict(
    :loop_frequency => 10,   # Hz
    :loop_function  => loop,
)

config_str = Goby.read_cli_cfg()
app = BasicJuliaPublisher(config_str)
Goby.run(app)
