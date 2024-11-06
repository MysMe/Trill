add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

add_requires("curlpp", "nlohmann_json", "osmanip", "glfw", "rmlui", "glad")

set_languages("c++20")

target("Trill")
    set_default(true)
    set_exceptions("cxx")
    set_kind("binary")
    add_includedirs("include", "rmlui")
    add_headerfiles("include/**.h", "rmlui/*.h")
    add_files("src/*.cpp", "rmlui/*.cpp")
    add_packages("curlpp", "nlohmann_json", "osmanip", "glfw", "rmlui", "glad")
    set_warnings("allextra", "error")
    if is_mode("debug") then
        add_defines("PROJECT_DIR=\"$(projectdir)\"")
    else
        add_defines("PROJECT_DIR=\"\"")
        if is_plat("windows") then
            add_ldflags("-subsystem:windows")
        end
    end
    