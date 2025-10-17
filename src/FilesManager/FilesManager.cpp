#include "FilesManager.hpp"

FilesManager::FilesManager(std::vector<fs::path> paths, bool recursive,
                           std::vector<std::string> defaultHeaderExtensions,
                           std::vector<std::string> defaultExcludeDirs) {}

FilesManager::FilesManager(fs::path configPath) {}

auto FilesManager::isExcludedDir(const fs::path &dirPath) -> bool {
  return std::any_of(excludeDirs_.begin(), excludeDirs_.end(),
                     [&](const std::string &excludedDir) { return dirPath.filename() == excludedDir; });
}

auto FilesManager::hasHeaderExtension(const fs::path &filePath) -> bool {
  return std::any_of(headerExtensions_.begin(), headerExtensions_.end(),
                     [&](const std::string &ext) { return filePath.extension() == ext; });
}

static auto getSizeRecursive(std::vector<fs::path> paths) -> uintmax_t {
  uintmax_t totalSize = 0;

  for (const auto &path : paths) {
    if (fs::is_regular_file(path)) {
      totalSize += fs::file_size(path);
      continue;
    }
    if (fs::is_directory(path)) {
      fs::directory_options options = fs::directory_options::skip_permission_denied;
      for (const auto &entry : fs::recursive_directory_iterator(path, options)) {
        if (fs::is_regular_file(entry.path())) {
          totalSize += fs::file_size(entry.path());
        }
      }
    }
  }
  return totalSize;
}

auto FilesManager::collectPathFiles(std::vector<fs::path> paths, bool recursive) -> std::vector<fs::path> {
  std::vector<fs::path> collectedFiles;
  uintmax_t filesCount = 0;
  uintmax_t totalSize = getSizeRecursive(paths);

  auto progressBar = bk::ProgressBar(&filesCount, {
                                                      .total = totalSize,
                                                      .message = "Collecting header files",
                                                      .speed = 1,
                                                      .speed_unit = "files/s",
                                                      .style = bk::ProgressBarStyle::Rich,
                                                  });

  for (const auto &path : paths) {
    if (!fs::exists(path)) {
      spdlog::warn("Path does not exist: {}", path.string());
      continue;
    }

    if (fs::is_regular_file(path) && hasHeaderExtension(path)) {
      collectedFiles.push_back(path);
      filesCount++;
      continue;
    }

    if (!fs::is_directory(path)) {
      fs::directory_options options = fs::directory_options::skip_permission_denied;
      if (recursive) {
        for (const auto &entry : fs::recursive_directory_iterator(path, options)) {
          if (fs::is_directory(entry.path()) && isExcludedDir(entry.path())) continue;
          if (fs::is_regular_file(entry.path()) && hasHeaderExtension(entry.path())) {
            collectedFiles.push_back(entry.path());
            filesCount++;
          }
        }
        continue;
      }
      for (const auto &entry : fs::directory_iterator(path, options)) {
        if (fs::is_directory(entry.path()) && isExcludedDir(entry.path())) continue;
        if (fs::is_regular_file(entry.path()) && hasHeaderExtension(entry.path())) {
          collectedFiles.push_back(entry.path());
          filesCount++;
        }
      }
    }
  }
  progressBar->done();
  return collectedFiles;
}

auto FilesManager::loadConfig() -> std::expected<void, std::string> {
  if (!fs::exists(configPath_)) return std::unexpected("Config file does not exist");
  json::json configJson = json::json::parse(configPath_.string(), nullptr, true);

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
}