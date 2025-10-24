#ifndef FILESMANAGER_HPP_
#define FILESMANAGER_HPP_

#include <barkeep/barkeep.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <expected>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>

#include "ObjectsManager/Object.hpp"

using namespace std::chrono_literals;
namespace fs = std::filesystem;
namespace json = nlohmann;
namespace bk = barkeep;

class FilesManager {
 public:
  FilesManager(fs::path configPath, bool noSave, std::vector<std::string> paths,
               std::vector<std::string> defaultHeaderExtensions, std::vector<std::string> defaultExcludeDirs,
               bool recursive);
  ~FilesManager() = default;
  auto getSourcePaths() const -> std::vector<fs::path>;
  auto getSavedObjects() -> std::vector<Object>;
  auto saveConfig(std::vector<Object> objects = {}) -> std::expected<void, std::string>;

 private:
  auto loadConfig() -> std::expected<void, std::string>;
  auto isExcludedDir(const fs::path &dirPath) -> bool;
  auto hasHeaderExtension(const fs::path &filePath) -> bool;
  auto collectPathFiles(std::vector<fs::path> paths) -> std::expected<std::vector<fs::path>, std::string>;

  bool recursive_;
  bool noSave_;
  std::vector<fs::path> sourcePaths_;
  fs::path configPath_;
  std::vector<std::string> excludeDirs_;
  std::vector<std::string> headerExtensions_;
  std::vector<Object> objects_;
};

#endif /* !FILESMANAGER_HPP_ */