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

using Goby

function init_cxxwrap(app_name)
    lib_path_expr = :(joinpath(@__DIR__, $app_name, "lib"*$app_name*".so"))
    @eval Goby begin
        @wrapmodule(() -> $lib_path_expr)
        function __init__()
            @initcxx
        end
    end
end

module Test
macro include_protos()
    return :(Base.include(Base.@__MODULE__, "/home/toby/opensource/goby3-examples/build/julia/goby3_examples/goby3_examples.jl"))
end
end
#Base.include(Base.@__MODULE__, joinpath(@__DIR__, "protobuf", "protobuf.jl"))

