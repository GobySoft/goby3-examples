# Julia subscriber equivalent to basic_subscriber.
#
# Run with:
#   julia --project=<build>/julia/Goby.jl subscriber.jl <config.pb.cfg>
#
# or use the CMake-generated launch wrapper which sets the correct --project.

using Pkg
# Activate the Goby.jl project located one level above this script in the
# build tree so that CxxWrap, ProtoBuf, etc. are available.
Pkg.activate(joinpath(@__DIR__, "..", "Goby.jl"), io=devnull)

# Load the generated C++ shared library.  It exposes BasicJuliaSubscriber
# together with cxx_run / cxx_publish / cxx_subscribe / cxx_set_loop_frequency_hertz
# and the INTERPROCESS / PROTOBUF / NULL_SCHEME constants.
using CxxWrap
@wrapmodule(() -> joinpath(@__DIR__, "libbasic_julia_subscriber"))
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
# start() is called by Goby.run() before entering the event loop.
# Set up subscriptions here.
# ---------------------------------------------------------------------------
function start()
    Goby.subscribe(Main.app, Goby.INTERPROCESS, "navigation",
        (nav::NavigationReport) -> begin
            println("Rx: x=$(nav.x) y=$(nav.y) z=$(nav.z)")
        end
    )
end

# ---------------------------------------------------------------------------
# Main – read configuration from the command-line config file, create the
# application object and hand control to Goby.run().
# ---------------------------------------------------------------------------
config_str = Goby.read_cli_cfg()
app = BasicJuliaSubscriber(config_str)
Goby.run(app)
