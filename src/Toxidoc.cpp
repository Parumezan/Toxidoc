#include <cxxopts.hpp>

#include "FilesManager/FilesManager.hpp"
#include "ObjectsManager/ObjectsManager.hpp"

static auto processDocumentationStatus(const std::vector<Object> &objects, bool verbose = false) -> int {
  uintmax_t undocumentedCount = 0;
  std::string statusReport;
  for (auto &obj : objects) {
    if (obj.isValid()) continue;
    if (!obj.isValid() && obj.getState() != ObjectState::Removed) {
      if (verbose)
        spdlog::warn("{} {} - Undocumented", obj.getObjectPathAsString(), obj.getStateAsString());
      else
        std::cout << obj.getObjectPathAsString() << " " << obj.getObjectTypeAsString() << " " << obj.getObjectName()
                  << " " << obj.getStateAsString() << std::endl;
      undocumentedCount++;
      continue;
    }
    if (verbose) spdlog::info("{} {}", obj.getObjectPathAsString(), obj.getStateAsString());
  }
  undocumentedCount > 0 ? spdlog::info("{}/{} objects are undocumented.", undocumentedCount, objects.size())
                        : spdlog::info("All {} objects are documented.", objects.size());
  return undocumentedCount > 0 ? 1 : 0;
}

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
      cxxopts::value<std::vector<std::string>>()->default_value("build,.git,third_party,external"))(
      "v,verbose", "Enable verbose logging to see detailed processing information",
      cxxopts::value<bool>()->default_value("false"))("h,help", "Print usage");

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

  ObjectsManager objectsManager;

  std::vector<fs::path> sourcePaths = filesManager.getSourcePaths();
  for (const auto &path : sourcePaths) {
    auto processResult = objectsManager.processHeaderFile(path);
    if (!processResult) {
      spdlog::error("Error processing file {}: {}", path.string(), processResult.error());
      continue;
    }
  }
  const auto &parsedObjects = objectsManager.getObjectsList();
  const auto &savedObjects = filesManager.getSavedObjects();

  if (savedObjects.empty()) {
    if (!result["no-save"].as<bool>()) {
      auto saveResult = filesManager.saveConfig(parsedObjects);
      if (!saveResult) {
        spdlog::error("Failed to save config: {}", saveResult.error());
        return 1;
      }
    }
    spdlog::info("Saved {} objects to config, processing documentation status.", parsedObjects.size());
    return processDocumentationStatus(parsedObjects, result["verbose"].as<bool>());
  }

  std::vector<Object> mergedObjects;
  for (const auto &savedObj : savedObjects) {
    bool found = false;
    for (const auto &parsedObj : parsedObjects) {
      if (savedObj == parsedObj) {
        found = true;
        break;
      }
    }
    if (!found) {
      mergedObjects.push_back(savedObj);
      mergedObjects.back().setState(ObjectState::Removed);
      continue;
    }
  }
  for (const auto &parsedObj : parsedObjects) {
    bool found = false;
    for (const auto &savedObj : savedObjects) {
      if (parsedObj == savedObj) {
        found = true;
        Object updatedObj = savedObj;
        updatedObj.updateObject(parsedObj);
        mergedObjects.push_back(updatedObj);
        break;
      }
    }
    if (!found) {
      mergedObjects.push_back(parsedObj);
      mergedObjects.back().setState(ObjectState::Added);
    }
  }
  return processDocumentationStatus(mergedObjects, result["verbose"].as<bool>());
}