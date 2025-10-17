set_project("Toxidoc")
set_version("0.1.0")

add_rules("mode.debug", "mode.release")

add_requires("nlohmann_json", "spdlog", "barkeep")

target("toxidoc")
    set_kind("binary")
    set_languages("cxx23")
    add_files("src/**.cpp")
    add_includedirs("src")
    add_syslinks("LLVM-20")
    add_packages("nlohmann_json", "spdlog", "barkeep")