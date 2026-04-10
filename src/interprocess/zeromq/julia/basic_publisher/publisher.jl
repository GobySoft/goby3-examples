# Julia publisher equivalent to basic_publisher.
#
# Run with:
#   julia --project=<build>/julia/Goby.jl publisher.jl <config.pb.cfg>
#
# or use the CMake-generated launch wrapper which sets the correct --project.

using Pkg
# Activate the Goby.jl project located one level above this script in the
# build tree so that CxxWrap, ProtoBuf, etc. are available.
Pkg.activate(joinpath(@__DIR__, "..", "Goby.jl"), io=devnull)

# Load the generated C++ shared library.  It exposes BasicJuliaPublisher
# together with cxx_run / cxx_publish / cxx_subscribe / cxx_set_loop_frequency_hertz
# and the INTERPROCESS / PROTOBUF / NULL_SCHEME constants.
using CxxWrap
@wrapmodule(() -> joinpath(@__DIR__, "libbasic_julia_publisher"))
function __init__()
    @initcxx
end

# Load the Goby Julia framework (higher-level publish / subscribe / run helpers).
using Goby

# Load the Julia protobuf bindings generated from nav.proto by ProtoBuf.protojl().
# nav.proto declares `package protobuf`, so protojl creates a module named
# "protobuf" inside protobuf/protobuf.jl which in turn includes nav_pb.jl.
include(joinpath(@__DIR__, "..", "protobuf", "protobuf.jl"))
using .protobuf: NavigationReport

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

# ---------------------------------------------------------------------------
# Main – read configuration from the command-line config file, create the
# application object and hand control to Goby.run().
# ---------------------------------------------------------------------------
config_str = Goby.read_cli_cfg()
app = BasicJuliaPublisher(config_str)
Goby.run(app)
