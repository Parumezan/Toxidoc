#include "FilesManager.hpp"

FilesManager::FilesManager(fs::path configPath, bool noSave, std::vector<std::string> paths,
                           std::vector<std::string> defaultHeaderExtensions,
                           std::vector<std::string> defaultExcludeDirs, std::vector<std::string> wordsBlacklist,
                           std::vector<std::string> typesBlacklist, bool recursive)
    : configPath_(configPath),
      noSave_(noSave),
      recursive_(recursive),
      headerExtensions_(defaultHeaderExtensions),
      excludeDirs_(defaultExcludeDirs),
      wordsBlacklist_(wordsBlacklist),
      typesBlacklist_(typesBlacklist),
      objects_({}) {
  for (const auto &pathStr : paths) sourcePaths_.push_back(fs::path(pathStr));
}

auto FilesManager::init() -> std::expected<void, std::string> {
  bool tryConfig = false;
  if (!configPath_.empty()) {
    tryConfig = true;
    auto loadResult = loadConfig();
    if (loadResult) return {};
    spdlog::warn("Failed to load config: {}", loadResult.error());
    spdlog::info("Using provided parameters and defaults");
    configPath_ = "toxiconf.json";
    loadResult = loadConfig();
    if (loadResult) return {};
    spdlog::warn("Failed to load default config: {}", loadResult.error());
  }
  if (sourcePaths_.empty() && !tryConfig) {
    spdlog::warn("No source paths provided, trying to load from default config");
    configPath_ = "toxiconf.json";
    auto loadResult = loadConfig();
    if (loadResult) return {};
    spdlog::warn("Failed to load default config: {}", loadResult.error());
  }
  if (sourcePaths_.empty()) {
    spdlog::warn("No source paths associated, using current directory");
    sourcePaths_.push_back(fs::current_path());
  }
  auto collectResult = collectPathFiles(sourcePaths_);
  if (!collectResult) return std::unexpected(collectResult.error());
  sourcePaths_ = collectResult.value();
  if (configPath_.empty()) configPath_ = "toxiconf.json";
  return {};
}

auto FilesManager::getSourcePaths() const -> std::vector<fs::path> { return sourcePaths_; }

auto FilesManager::getSavedObjects() -> std::vector<Object> { return objects_; }

auto FilesManager::getWordsBlacklist() const -> std::vector<std::string> { return wordsBlacklist_; }

auto FilesManager::getTypesBlacklist() const -> std::vector<std::string> { return typesBlacklist_; }

auto FilesManager::getLastSaveTime() const -> std::chrono::system_clock::time_point { return lastSaveTime_; }

auto FilesManager::saveConfig(std::vector<Object> objects) -> std::expected<void, std::string> {
  if (configPath_.empty()) return std::unexpected("Config path is empty");

  json::json configJson;
  configJson["exclude_dirs"] = excludeDirs_;
  configJson["header_extensions"] = headerExtensions_;
  configJson["words_blacklist"] = wordsBlacklist_;
  configJson["types_blacklist"] = typesBlacklist_;

  configJson["last_saved"] =
      std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

  std::vector<std::string> sourcePathsStr;
  for (const auto &path : sourcePaths_) sourcePathsStr.push_back(path.string());
  configJson["source_paths"] = sourcePathsStr;

  std::vector<json::json> objectsJson;
  for (const auto &obj : objects)
    if (obj.getState() != ObjectState::Removed) objectsJson.push_back(obj.getObjectAsJSON());
  configJson["objects"] = objectsJson;

  std::ofstream configFile(configPath_);
  if (!configFile.is_open()) return std::unexpected("Failed to open config file for writing");
  configFile << configJson.dump(4);
  configFile.close();
  return {};
}

auto FilesManager::loadConfig() -> std::expected<void, std::string> {
  if (!fs::exists(configPath_)) return std::unexpected("Config file does not exist");
  std::ifstream configFile(configPath_);
  if (!configFile.is_open()) return std::unexpected("Failed to open config file");
  json::json configJson = json::json::parse(configFile, nullptr, false);
  if (configJson.is_discarded()) return std::unexpected("Failed to parse config file");
  spdlog::info("Loading config from {}", configPath_.string());

  if (configJson.contains("last_saved") && configJson["last_saved"].is_number_unsigned()) {
    auto lastSavedSeconds = configJson["last_saved"].get<uint64_t>();
    lastSaveTime_ = std::chrono::system_clock::time_point(std::chrono::seconds(lastSavedSeconds));
  }
  if (configJson.contains("exclude_dirs") && configJson["exclude_dirs"].is_array()) {
    excludeDirs_.clear();
    for (const auto &dir : configJson["exclude_dirs"])
      if (dir.is_string()) excludeDirs_.push_back(dir.get<std::string>());
  }
  if (configJson.contains("header_extensions") && configJson["header_extensions"].is_array() &&
      headerExtensions_.empty()) {
    headerExtensions_.clear();
    for (const auto &ext : configJson["header_extensions"])
      if (ext.is_string()) headerExtensions_.push_back(ext.get<std::string>());
  }
  if (configJson.contains("words_blacklist") && configJson["words_blacklist"].is_array()) {
    wordsBlacklist_.clear();
    for (const auto &word : configJson["words_blacklist"])
      if (word.is_string()) wordsBlacklist_.push_back(word.get<std::string>());
  }
  if (configJson.contains("types_blacklist") && configJson["types_blacklist"].is_array()) {
    typesBlacklist_.clear();
    for (const auto &type : configJson["types_blacklist"])
      if (type.is_string()) typesBlacklist_.push_back(type.get<std::string>());
  }
  if (configJson.contains("source_paths") && configJson["source_paths"].is_array()) {
    sourcePaths_.clear();
    for (const auto &path : configJson["source_paths"])
      if (path.is_string()) sourcePaths_.push_back(fs::path(path.get<std::string>()));
  }
  if (sourcePaths_.empty()) return std::unexpected("No source paths found in config");

  if (configJson.contains("objects") && configJson["objects"].is_array()) {
    objects_.clear();
    for (const auto &obj : configJson["objects"])
      if (obj.is_object()) objects_.emplace_back(Object(obj));
    if (!objects_.empty()) spdlog::info("Loaded {} objects from config", objects_.size());
  }
  return {};
}

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
  std::atomic<size_t> filesCount = 0;

  auto status = bk::Status({
      .message = "Collecting source files...",
      .style = bk::AnimationStyle::Bar,
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