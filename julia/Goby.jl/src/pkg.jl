import Pkg;
function install_pkgs()
    Pkg.instantiate()
    Pkg.add("Pkg")
    Pkg.add("ProtoBuf")
    Pkg.add("CxxWrap")
    Pkg.add("YAML")
    Pkg.add("ThreadPools")
end
