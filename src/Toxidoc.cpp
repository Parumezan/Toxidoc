#include <cxxopts.hpp>
#include <iostream>

#include "FilesManager/FilesManager.hpp"
#include "ObjectsManager/ObjectsManager.hpp"

static auto processDocumentationStatus(const std::vector<Object> &objects, bool lite_verbose = false) -> int {
  size_t undocumentedCount = 0;
  std::string statusReport;

  size_t totalSize = 0;
  for (const auto &obj : objects) {
    if (obj.getState() == ObjectState::Removed) continue;
    totalSize++;
  }

  for (auto &obj : objects) {
    if (obj.isValid() && lite_verbose) continue;
    if (!obj.isValid() && obj.getState() != ObjectState::Removed) {
      if (lite_verbose) {
        std::cout << obj.getObjectPathAsString() << " " << obj.getObjectTypeAsString() << " " << obj.getObjectName()
                  << std::endl;
      } else {
        spdlog::warn("{} {} {} {}", obj.getObjectPathAsString(), obj.getObjectTypeAsString(), obj.getObjectName(),
                     obj.getStateAsString());
      }
      undocumentedCount++;
      continue;
    }
    if (!lite_verbose) {
      spdlog::info("{} {} {} {}", obj.getObjectPathAsString(), obj.getObjectTypeAsString(), obj.getObjectName(),
                   obj.getStateAsString());
    }
  }
  undocumentedCount > 0 ? spdlog::info("{}/{} objects are documented, {} undocumented left",
                                       totalSize - undocumentedCount, totalSize, undocumentedCount)
                        : spdlog::info("All {} objects are documented", totalSize);
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
      "l,lite-verbose", "Lite verbose output mode", cxxopts::value<bool>()->default_value("false"))("h,help",
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

  auto initResult = filesManager.init();
  if (!initResult) {
    spdlog::error("Failed to initialize FilesManager: {}", initResult.error());
    return 1;
  }

  ObjectsManager objectsManager;

  size_t processedFiles = 0;
  auto status = bk::ProgressBar(&processedFiles, {
                                                     .total = filesManager.getSourcePaths().size(),
                                                     .message = "Processing objects in files...",
                                                     .interval = 1.0,
                                                     .no_tty = true,
                                                 });

  std::vector<fs::path> sourcePaths = filesManager.getSourcePaths();
  for (const auto &path : sourcePaths) {
    processedFiles++;
    auto processResult = objectsManager.processHeaderFile(path);
    if (!processResult) {
      spdlog::error("Error processing file {}: {}", path.string(), processResult.error());
      continue;
    }
  }
  status->done();

  const auto &parsedObjects = objectsManager.getObjectsList();
  const auto &savedObjects = filesManager.getSavedObjects();

  if (savedObjects.empty()) {
    if (!result["no-save"].as<bool>()) {
      auto saveResult = filesManager.saveConfig(parsedObjects);
      if (!saveResult) {
        spdlog::error("Failed to save config: {}", saveResult.error());
        return 1;
      }
      spdlog::info("Saved {} objects to config", parsedObjects.size());
    }
    return processDocumentationStatus(parsedObjects, result["lite-verbose"].as<bool>());
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
  if (!result["no-save"].as<bool>()) {
    auto saveResult = filesManager.saveConfig(mergedObjects);
    if (!saveResult) {
      spdlog::error("Failed to save config: {}", saveResult.error());
      return 1;
    }
  }
  return processDocumentationStatus(mergedObjects, result["lite-verbose"].as<bool>());
}