# Julia publisher equivalent to basic_publisher.
#
# Publishes NavigationReport messages at 10 Hz on the "navigation" group.
#
# Run with:
#   julia build/julia/basic_publisher/publisher.jl <config.pb.cfg>

include(joinpath(@__DIR__, "..", "goby_application_init.jl"))
init_cxxwrap("basic_julia_publisher")
Test.@include_protos()

# ---------------------------------------------------------------------------
# Publisher loop – called at the frequency specified in goby_cfg below.
# ---------------------------------------------------------------------------
function loop()
    nav = goby3_examples.protobuf.NavigationReport(
        x = 95.0 + rand() * 20,
        y = 195.0 + rand() * 20,
        z = -305.0 + rand() * 10,
    )
    println(string(typeof(nav)))
    
    println("Tx: x=$(nav.x) y=$(nav.y) z=$(nav.z)")
    Goby.publish(Main.app, Goby.INTERPROCESS, "groups::nav2", nav)
end

# goby_cfg is inspected by Goby.run() to configure the loop frequency.
goby_cfg = Dict(
    :loop_frequency => 10,   # Hz
    :loop_function  => loop,
)

config_str = Goby.read_cli_cfg()
app = Goby.BasicJuliaPublisher(config_str)
Goby.run(app)
