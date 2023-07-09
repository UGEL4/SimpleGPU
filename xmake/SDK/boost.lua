boost_includes_dir = "$(projectdir)/SimpleGPU/SDK/boost"
target("boost")
    set_group("00.sdk")
    set_kind("headeronly")
    add_includedirs(boost_includes_dir, {public = true})