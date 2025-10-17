#include <cxxopts.hpp>

#include "FilesManager/FilesManager.hpp"

int main(int ac, char **av) {
  cxxopts::Options options("Toxidoc", "C++ Documentation Manager");

  options.add_options()("c,config", "Path to config file", cxxopts::value<std::string>()->default_value(""))(
      "r,recursive", "Recursively search directories", cxxopts::value<bool>()->default_value("true"))(
      "h,header_extensions", "Header file extensions (comma separated)",
      cxxopts::value<std::string>()->default_value(".h,.hpp,.hh,.hxx,.ipp,.tpp,.inl"))(
      "e,exclude_dirs", "Directories to exclude (comma separated)",
      cxxopts::value<std::string>()->default_value("build,.git,third_party,external"))("help", "Print usage");

  auto result = options.parse(ac, av);

  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    return 0;
  }

  std::string configPath = result["config"].as<std::string>();
  bool recursive = result["recursive"].as<bool>();

  if (!configPath.empty()) {
    FilesManager filesManager(fs::path(configPath));
    return 0;
  }
}