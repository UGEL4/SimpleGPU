soluton_dir     = "$(projectdir)"
project_name    = "SimpleGPU"
gpu_target_name = "SimpleGPU"
option("is_clang")
    add_cxxsnippets("is_clang", 'if(__clang__) return 0;', {tryrun = true})
option_end()
option("is_msvc")
    add_cxxsnippets("is_msvc", 'if(_MSC_VER) return 0;', {tryrun = true})
option_end()

project_ldflags = {}
project_cxflags = {}
project_mxflags = {}

-- uses utf-8 charset at runtime
if is_host("windows") then
    table.insert(project_cxflags, "/execution-charset:utf-8")
    table.insert(project_cxflags, "/source-charset:utf-8")
end

if(has_config("is_clang")) then
    table.insert(project_cxflags, "-Wno-unused-command-line-argument")
    table.insert(project_cxflags, "-Wno-format")
    -- table.insert(project_cxflags, "-Wno-deprecated-builtins")
    table.insert(project_cxflags, "-Wno-switch")
    table.insert(project_cxflags, "-Wno-misleading-indentation")
    table.insert(project_cxflags, "-Wno-unknown-pragmas")
    table.insert(project_cxflags, "-Wno-unused-function")
    table.insert(project_cxflags, "-Wno-ignored-attributes")
    table.insert(project_cxflags, "-Wno-deprecated-declarations")
    table.insert(project_cxflags, "-Wno-nullability-completeness")
    table.insert(project_cxflags, "-Wno-tautological-undefined-compare")
    table.insert(project_cxflags, "-Werror=return-type")
    -- enable time trace with clang compiler
    table.insert(project_cxflags, "-ftime-trace")
    if(has_config("is_msvc")) then
        table.insert(project_cxflags, "-Wno-microsoft-cast")
        table.insert(project_cxflags, "-Wno-microsoft-enum-forward-reference")
        if (is_mode("asan")) then
            table.insert(project_ldflags, "-fsanitize=address")
        end
    end
end

if(has_config("is_msvc")) then
    table.insert(project_ldflags, "/IGNORE:4217,4286")
    table.insert(project_cxflags, "/Zc:__cplusplus")
    table.insert(project_cxflags, "/FC")
    table.insert(project_cxflags, "/GR-")
    table.insert(project_cxflags, "/wd4251")
    if (is_mode("asan")) then
        table.insert(project_ldflags, "/fsanitize=address")
    end
end

set_project(project_name)
set_arch("x64")
set_warnings("all")
set_languages("c++20")
--set_toolchains("clang")
add_rules("mode.debug", "mode.release")

include_dir_list = {"$(projectdir)/"..project_name.."/Source/include", "$(projectdir)/"..project_name.."/SDK/vulkan", "$(projectdir)/"..project_name.."/SDK/boost"}
source_file_list = {"$(projectdir)/"..project_name.."/Source/src/build.**.cpp", "$(projectdir)/"..project_name.."/Source/src/shader-reflections/spirv/spirv_reflect.c",
"$(projectdir)/"..project_name.."/Source/include/render_graph/src/**.cpp"}
target(gpu_target_name)
    set_kind("binary")
    if (is_os("windows")) then 
        add_syslinks("advapi32", "user32", "shell32", "Ole32", {public = true})
    end
    add_cxflags(project_cxflags, {public = true, force = true})
    add_defines("UNICODE")
    add_defines("GPU_USE_VULKAN")
    add_defines("GPU_USE_D3D12")
    add_includedirs(include_dir_list)
    add_files(source_file_list, "$(projectdir)/"..project_name.."/Source/test/*.cpp")
target_end()

includes(project_name.."/Source/test/render_graph")