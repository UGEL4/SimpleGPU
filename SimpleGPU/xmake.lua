--local include_dir_list = {"$(projectdir)/"..project_name.."/Source/include", "$(projectdir)/"..project_name.."/SDK/vulkan", "$(projectdir)/"..project_name.."/SDK/boost"}
local source_file_list = {"$(projectdir)/"..project_name.."/Source/src/build.**.cpp", "$(projectdir)/"..project_name.."/Source/src/shader-reflections/spirv/spirv_reflect.c",
"$(projectdir)/"..project_name.."/Source/include/render_graph/src/**.cpp"}
target("SimpleGPU")
    set_kind("headeronly")
    add_deps("vulkan", "boost")
    if (is_os("windows")) then 
        add_syslinks("advapi32", "user32", "shell32", "Ole32", {public = true})
    end
    add_cxflags(project_cxflags, {public = true, force = true})
    add_defines("UNICODE")
    add_defines("GPU_USE_VULKAN")
    add_defines("GPU_USE_D3D12")
    add_includedirs(include_dir_list , {public = true})
    --[[ add_headerfiles("$(projectdir)/SimpleGPU/(Source/include/**.h)", "$(projectdir)/SimpleGPU/(Source/include/**.hpp)", "$(projectdir)/SimpleGPU/(Source/include/**.cpp)",
    "$(projectdir)/SimpleGPU/(Source/src/**.c)",
    "$(projectdir)/SimpleGPU/(Source/src/**.cpp)", {install = false}) ]]
    --add_files(source_file_list--[[ , "$(projectdir)/"..project_name.."/Source/test/*.cpp" ]])