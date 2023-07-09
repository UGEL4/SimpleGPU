local source_file_list = {"$(projectdir)/"..project_name.."/Source/src/build.**.cpp", "$(projectdir)/"..project_name.."/Source/src/shader-reflections/spirv/spirv_reflect.c",
"$(projectdir)/"..project_name.."/Source/include/render_graph/src/**.cpp"}
target("test_simple_gpu")
    set_kind("binary")
    add_deps("SimpleGPU")
    set_group("test/simple_gpu")
    add_defines("UNICODE")
    add_files("main.cpp", source_file_list)