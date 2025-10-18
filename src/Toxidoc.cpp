#include <cxxopts.hpp>

#include "FilesManager/FilesManager.hpp"

int main(int ac, char **av) {
  cxxopts::Options options("Toxidoc", "C++ Documentation Manager");

  options.add_options()("c,config", "Path to config file. If you provide this option, it will erase next parameters.",
                        cxxopts::value<std::string>())("n,no-save", "Do not save config file after initialization",
                                                       cxxopts::value<bool>()->default_value("false"))(
      "s,source-paths", "Source paths (comma separated)", cxxopts::value<std::vector<std::string>>())(
      "r,recursive", "Recursively search directories", cxxopts::value<bool>()->default_value("true"))(
      "x,header-extensions", "Header file extensions (comma separated)",
      cxxopts::value<std::vector<std::string>>()->default_value(".h,.hpp,.hh,.hxx,.ipp,.tpp,.inl"))(
      "e,exclude-dirs", "Directories to exclude (comma separated)",
      cxxopts::value<std::vector<std::string>>()->default_value("build,.git,third_party,external"))("h,help",
                                                                                                    "Print usage");

  auto result = options.parse(ac, av);

  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    return 0;
  }

  FilesManager filesManager(
      result.count("config") ? fs::path(result["config"].as<std::string>()) : fs::path(), result["no-save"].as<bool>(),
      result.count("source-paths") ? result["source-paths"].as<std::vector<std::string>>() : std::vector<std::string>{},
      result["header-extensions"].as<std::vector<std::string>>(), result["exclude-dirs"].as<std::vector<std::string>>(),
      result["recursive"].as<bool>());

  std::vector<fs::path> sourcePaths = filesManager.getSourcePaths();
  for (const auto &path : sourcePaths) {
    std::cout << "Source Path: " << path.string() << std::endl;
  }

  return 0;
}