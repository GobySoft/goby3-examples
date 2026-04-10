# Shared initialization for Goby interprocess Julia example applications.
#
# Before including this file, define GOBY_APPLICATION_LIB as the full path
# to the application shared library (without file extension), e.g.:
#   const GOBY_APPLICATION_LIB = joinpath(@__DIR__, "libbasic_julia_publisher")
#
# After returning from this include the following are available:
#   - All CxxWrap'd symbols from the application library
#     (e.g. BasicJuliaPublisher, INTERPROCESS, PROTOBUF, ...)
#   - Goby module  (Goby.publish, Goby.subscribe, Goby.run, ...)
#   - NavigationReport struct (from nav.proto Julia bindings)

using Pkg
Pkg.activate(joinpath(@__DIR__, "Goby.jl"), io=devnull)

using CxxWrap
@wrapmodule(() -> GOBY_APPLICATION_LIB)
function __init__()
    @initcxx
end

using Goby

include(joinpath(@__DIR__, "protobuf", "protobuf.jl"))
using .protobuf: NavigationReport
