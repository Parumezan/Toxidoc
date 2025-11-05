#include <cxxopts.hpp>
#include <iostream>

#include "FilesManager/FilesManager.hpp"
#include "ObjectsManager/ObjectsManager.hpp"
#include "Utils.hpp"

static auto showCoverageBar(const std::vector<Object> &objects) -> void {
  spdlog::info("Documentation coverage per file:");

  std::map<std::string, std::vector<Object>> docsByFile;
  for (const auto &obj : objects) docsByFile[obj.getObjectPath().string()].push_back(obj);
  size_t allTotalObjects = 0;
  size_t allDocumentedObjects = 0;
  size_t overallTitleOffset = 0;

  for (const auto &[filePath, objs] : docsByFile)
    if (filePath.length() > overallTitleOffset) overallTitleOffset = filePath.length();

  for (const auto &[filePath, objs] : docsByFile) {
    size_t totalObjects = 0;
    size_t documentedObjects = 0;
    for (const auto &obj : objs) {
      if (obj.getState() == ObjectState::Removed) continue;
      totalObjects++;
      allTotalObjects++;
      if (obj.isValid()) {
        documentedObjects++;
        allDocumentedObjects++;
      }
    }

    auto status =
        bk::ProgressBar(&documentedObjects, {
                                                .total = totalObjects,
                                                .message = fmt::format("{:<{}}", filePath, overallTitleOffset),
                                                .style = bk::ProgressBarStyle::Rich,
                                                .no_tty = true,
                                                .show = true,
                                            });
    status->done();
    cleanupProgressBar();
    cleanupProgressBar();
  }
  auto overallStatus = bk::ProgressBar(&allDocumentedObjects,
                                       {
                                           .total = allTotalObjects,
                                           .message = fmt::format("{:<{}}", "Overall coverage", overallTitleOffset),
                                           .style = bk::ProgressBarStyle::Rich,
                                           .no_tty = true,
                                           .show = true,
                                       });
  overallStatus->done();
  cleanupProgressBar();
  cleanupProgressBar();
}

static auto processDocumentationStatus(const std::vector<Object> &objects, bool verbose, bool coverage) -> int {
  size_t undocumentedCount = 0;
  std::string statusReport;

  size_t totalSize = 0;
  for (const auto &obj : objects) {
    if (obj.getState() == ObjectState::Removed) continue;
    totalSize++;
  }

  for (auto &obj : objects) {
    if (obj.isValid() && verbose) continue;
    if (!obj.isValid() && obj.getState() != ObjectState::Removed) {
      if (verbose) {
        spdlog::warn("{} {} {} {}", obj.getObjectPathAsString(), obj.getObjectTypeAsString(), obj.getObjectName(),
                     obj.getStateAsString());
      } else {
        std::cout << obj.getObjectPathAsString() << " " << obj.getObjectTypeAsString() << " " << obj.getObjectName()
                  << std::endl;
      }
      undocumentedCount++;
      continue;
    }
    if (verbose) {
      spdlog::info("{} {} {} {}", obj.getObjectPathAsString(), obj.getObjectTypeAsString(), obj.getObjectName(),
                   obj.getStateAsString());
    }
  }
  if (coverage) {
    showCoverageBar(objects);
    return undocumentedCount > 0 ? 1 : 0;
  }
  undocumentedCount > 0 ? spdlog::info("{}/{} objects are documented, {} undocumented left",
                                       totalSize - undocumentedCount, totalSize, undocumentedCount)
                        : spdlog::info("All {} objects are documented", totalSize);
  return undocumentedCount > 0 ? 1 : 0;
}

int main(int ac, char **av) {
  cxxopts::Options options("Toxidoc", "C++ Documentation Manager");

  options.add_options()("c,config", "Path to config file", cxxopts::value<std::string>())(
      "n,no-save", "Do not save config file after initialization", cxxopts::value<bool>()->default_value("false"))(
      "s,source-paths", "Source paths (comma separated)", cxxopts::value<std::vector<std::string>>())(
      "r,recursive", "Recursively search directories", cxxopts::value<bool>()->default_value("true"))(
      "g,generate", "Generate beginning documentation blocks for undocumented objects",
      cxxopts::value<bool>()->default_value("false"))(
      "o,get-object", "Shows objects whose name matches the argument you provide", cxxopts::value<std::string>())(
      "x,header-extensions", "Header file extensions (comma separated)",
      cxxopts::value<std::vector<std::string>>()->default_value(".h,.hpp,.hh,.hxx,.ipp,.tpp,.inl"))(
      "e,exclude-dirs", "Directories to exclude (comma separated)",
      cxxopts::value<std::vector<std::string>>()->default_value("build,.git,third_party,external"))(
      "b,blacklist", "Words to designate names to ignore (comma separated)",
      cxxopts::value<std::vector<std::string>>()->default_value("Q_PROPERTY"))(
      "t,types", "Blacklist of object types to document (comma separated)",
      cxxopts::value<std::vector<std::string>>()->default_value(""))("type-list", "List of available object types")(
      "v,verbose", "Verbose *LITE* output mode", cxxopts::value<bool>()->default_value("false"))(
      "last-update", "Show the last update time of the documentation", cxxopts::value<bool>())(
      "d, coverage", "Remove the progress bar for documentation coverage",
      cxxopts::value<bool>()->default_value("false"))(
      "mod",
      "add module name for clang parsing (e.g. --mod path/to/modules/qt_override.h in this case we use a header to "
      "override QT macros, refers to mods folder to list all modules ; don't create your own module, the code is not "
      "ready for that)",
      cxxopts::value<std::string>())("h,help", "Print usage");

  cxxopts::ParseResult result;
  try {
    result = options.parse(ac, av);
  } catch (const std::exception &e) {
    std::cerr << "Error parsing options: " << e.what() << std::endl;
    return 1;
  }

  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    return 0;
  }

  if (result.count("type-list")) {
    std::cout << "Available object types to blacklist:" << std::endl;
    for (const auto &[type, name] : ObjectTypeStringMap) { std::cout << " - " << name << std::endl; }
    return 0;
  }

  bool coverageRequested = result["coverage"].as<bool>() == false;
  bool verboseRequested = result["verbose"].as<bool>() == false;

  FilesManager filesManager(
      result.count("config") ? fs::path(result["config"].as<std::string>()) : fs::path(), result["no-save"].as<bool>(),
      result.count("mod") ? fs::path(result["mod"].as<std::string>()) : fs::path(),
      result.count("source-paths") ? result["source-paths"].as<std::vector<std::string>>() : std::vector<std::string>{},
      result["header-extensions"].as<std::vector<std::string>>(), result["exclude-dirs"].as<std::vector<std::string>>(),
      result["blacklist"].as<std::vector<std::string>>(), result["types"].as<std::vector<std::string>>(),
      result["recursive"].as<bool>());

  auto initResult = filesManager.init();
  if (!initResult) {
    spdlog::error("Failed to initialize FilesManager: {}", initResult.error());
    return 1;
  }

  ObjectsManager objectsManager(filesManager.getWordsBlacklist(), filesManager.getTypesBlacklist(),
                                filesManager.getModulePath());

  spdlog::info("Processing {} source files...", filesManager.getSourcePaths().size());
  size_t processedFiles = 0;
  auto status = bk::ProgressBar(&processedFiles, {
                                                     .total = filesManager.getSourcePaths().size(),
                                                     .message = "Processing objects in files...",
                                                     .style = bk::ProgressBarStyle::Rich,
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
  cleanupProgressBar();

  auto lastUpdateTime = filesManager.getLastSaveTime();
  if (lastUpdateTime == std::chrono::system_clock::time_point{}) lastUpdateTime = std::chrono::system_clock::now();
  spdlog::info("Last documentation update: {}", getReadableTimeString(lastUpdateTime));

  const auto &parsedObjects = objectsManager.getObjectsList();
  const auto &savedObjects = filesManager.getSavedObjects();

  if (result.count("get-object")) {
    const std::string &objName = result["get-object"].as<std::string>();
    size_t foundCount = 0;
    for (const auto &obj : parsedObjects) {
      if (obj.getObjectName().contains(objName)) {
        foundCount++;
        std::cout << obj.getObjectAsString() << std::endl;
      }
    }
    spdlog::info("Found {} objects matching '{}'", foundCount, objName);
    return 0;
  }

  if (result["generate"].as<bool>()) {
    objectsManager.generateDocumentation();
    return 0;
  }

  if (savedObjects.empty()) {
    if (!result["no-save"].as<bool>()) {
      auto saveResult = filesManager.saveConfig(parsedObjects);
      if (!saveResult) {
        spdlog::error("Failed to save config: {}", saveResult.error());
        return 1;
      }
      spdlog::info("Saved {} objects to config", parsedObjects.size());
    }
    return processDocumentationStatus(parsedObjects, verboseRequested, coverageRequested);
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
  return processDocumentationStatus(mergedObjects, verboseRequested, coverageRequested);
}