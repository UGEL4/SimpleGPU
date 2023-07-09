vulkan_includes_dir = "$(projectdir)/SimpleGPU/SDK/vulkan"
target("vulkan")
    set_group("00.sdk")
    set_kind("headeronly")
    add_includedirs(vulkan_includes_dir, {public = true})