# Julia publisher equivalent to basic_publisher.
#
# Publishes NavigationReport messages at 10 Hz on the "groups::julia_nav" group.
#
# Run with:
#   julia build/julia/basic_julia_publisher/publisher.jl <config.pb.cfg>

include(joinpath(@__DIR__, "..", "goby_application_init.jl"))
init_cxxwrap("basic_julia_publisher")
include_protos(@__MODULE__, "goby3_examples")

# ---------------------------------------------------------------------------
# Publisher loop – called at the frequency specified in goby_cfg below.
# ---------------------------------------------------------------------------
function loop()
    nav = goby3_examples.protobuf.NavigationReport(
        x = 95.0 + rand() * 20,
        y = 195.0 + rand() * 20,
        z = -305.0 + rand() * 10,
    )

    println("Tx: x=$(nav.x) y=$(nav.y) z=$(nav.z)")
    # the group is given by its string value, which GOBY_DEFINE_GROUP sets to
    # the fully-qualified C++ name (see src/messages/groups.h)
    Goby.publish(Main.app, Goby.INTERPROCESS, "groups::julia_nav", nav)
end

# goby_cfg is inspected by Goby.run() to configure the loop frequency.
goby_cfg = Dict(
    :loop_frequency => 10,   # Hz
    :loop_function  => loop,
)

config_str = Goby.read_cli_cfg()
app = Goby.BasicJuliaPublisher(config_str)
Goby.run(app)
