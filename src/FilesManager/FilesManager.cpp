#include "FilesManager.hpp"

FilesManager::FilesManager(fs::path configPath, bool noSave, std::vector<std::string> paths,
                           std::vector<std::string> defaultHeaderExtensions,
                           std::vector<std::string> defaultExcludeDirs, bool recursive)
    : configPath_(configPath),
      noSave_(noSave),
      recursive_(recursive),
      headerExtensions_(defaultHeaderExtensions),
      excludeDirs_(defaultExcludeDirs) {
  for (const auto &pathStr : paths) sourcePaths_.push_back(fs::path(pathStr));
  if (!configPath_.empty()) {
    auto loadResult = loadConfig();
    if (loadResult) return;
    spdlog::warn("Failed to load config: {}", loadResult.error());
    spdlog::info("Using provided parameters and defaults");
  }
  configPath_ = "toxiconf.json";
  if (sourcePaths_.empty()) {
    spdlog::error("No source paths provided. Exiting.");
    return;
  }
  auto loadResult = loadConfig();
  if (loadResult) return;
  auto collectResult = collectPathFiles(sourcePaths_);
  if (!collectResult) {
    spdlog::error("Failed to collect source files: {}", collectResult.error());
    sourcePaths_.clear();
    return;
  }
  sourcePaths_ = collectResult.value();
  if (noSave_) return;
  auto saveResult = saveConfig();
  if (!saveResult) spdlog::error("Failed to save config: {}", saveResult.error());
}

auto FilesManager::getSourcePaths() const -> std::vector<fs::path> { return sourcePaths_; }

auto FilesManager::isExcludedDir(const fs::path &dirPath) -> bool {
  return std::any_of(excludeDirs_.begin(), excludeDirs_.end(),
                     [&](const std::string &excludedDir) { return dirPath.filename() == excludedDir; });
}

auto FilesManager::hasHeaderExtension(const fs::path &filePath) -> bool {
  return std::any_of(headerExtensions_.begin(), headerExtensions_.end(),
                     [&](const std::string &ext) { return filePath.extension() == ext; });
}

auto FilesManager::collectPathFiles(std::vector<fs::path> paths) -> std::expected<std::vector<fs::path>, std::string> {
  std::vector<fs::path> collectedFiles;
  std::atomic<uintmax_t> filesCount = 0;

  auto status = bk::Status({
      .message = "Collecting source files",
      .style = bk::AnimationStyle::Bounce,
      .show = true,
  });

  for (const auto &path : paths) {
    if (!fs::exists(path)) continue;
    if (fs::is_regular_file(path) && hasHeaderExtension(path)) {
      collectedFiles.push_back(path);
      filesCount++;
      status->message(fmt::format("Collecting source files (found {} files so far)", filesCount.load()));
      continue;
    }
    if (fs::is_directory(path)) {
      fs::directory_options options = fs::directory_options::skip_permission_denied;
      if (recursive_) {
        for (const auto &entry : fs::recursive_directory_iterator(path, options)) {
          if (fs::is_directory(entry.path()) && isExcludedDir(entry.path())) continue;
          if (fs::is_regular_file(entry.path()) && hasHeaderExtension(entry.path())) {
            collectedFiles.push_back(entry.path());
            filesCount++;
            status->message(fmt::format("Collecting source files (found {} files so far)", filesCount.load()));
          }
        }
        continue;
      }
      for (const auto &entry : fs::directory_iterator(path, options)) {
        if (fs::is_directory(entry.path()) && isExcludedDir(entry.path())) continue;
        if (fs::is_regular_file(entry.path()) && hasHeaderExtension(entry.path())) {
          collectedFiles.push_back(entry.path());
          filesCount++;
          status->message(fmt::format("Collecting source files (found {} files so far)", filesCount.load()));
        }
      }
    }
  }
  status->message(fmt::format("Collected {} source files", filesCount.load()));
  status->done();
  return collectedFiles;
}

auto FilesManager::loadConfig() -> std::expected<void, std::string> {
  if (!fs::exists(configPath_)) return std::unexpected("Config file does not exist");
  std::ifstream configFile(configPath_);
  if (!configFile.is_open()) return std::unexpected("Failed to open config file");
  json::json configJson = json::json::parse(configFile, nullptr, false);
  if (configJson.is_discarded()) return std::unexpected("Failed to parse config file");

  if (configJson.contains("exclude_dirs") && configJson["exclude_dirs"].is_array()) {
    excludeDirs_.clear();
    for (const auto &dir : configJson["exclude_dirs"])
      if (dir.is_string()) excludeDirs_.push_back(dir.get<std::string>());
  }
  if (configJson.contains("header_extensions") && configJson["header_extensions"].is_array()) {
    headerExtensions_.clear();
    for (const auto &ext : configJson["header_extensions"])
      if (ext.is_string()) headerExtensions_.push_back(ext.get<std::string>());
  }
  if (configJson.contains("source_paths") && configJson["source_paths"].is_array()) {
    sourcePaths_.clear();
    for (const auto &path : configJson["source_paths"])
      if (path.is_string()) sourcePaths_.push_back(fs::path(path.get<std::string>()));
  }
  if (sourcePaths_.empty()) return std::unexpected("No source paths found in config");
  return {};
}

auto FilesManager::saveConfig() -> std::expected<void, std::string> {
  if (configPath_.empty()) return std::unexpected("Config path is empty");

  json::json configJson;
  configJson["exclude_dirs"] = excludeDirs_;
  configJson["header_extensions"] = headerExtensions_;

  std::vector<std::string> sourcePathsStr;
  for (const auto &path : sourcePaths_) sourcePathsStr.push_back(path.string());
  configJson["source_paths"] = sourcePathsStr;

  std::ofstream configFile(configPath_);
  if (!configFile.is_open()) return std::unexpected("Failed to open config file for writing");
  configFile << configJson.dump(4);
  configFile.close();
  return {};
}