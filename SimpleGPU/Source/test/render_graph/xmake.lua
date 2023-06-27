target("render_graph_test")
    set_kind("binary")
    if (is_os("windows")) then 
        add_syslinks("advapi32", "user32", "shell32", "Ole32", {public = true})
    end
    --add_defines("GPU_USE_VULKAN")
    --add_defines("GPU_USE_D3D12")
    add_includedirs("$(projectdir)/"..project_name.."/Source/include", "$(projectdir)/"..project_name.."/SDK/boost")
    add_files("$(projectdir)/"..project_name.."/Source/test/render_graph/**.cpp", "$(projectdir)/"..project_name.."/Source/include/render_graph/src/**.cpp")