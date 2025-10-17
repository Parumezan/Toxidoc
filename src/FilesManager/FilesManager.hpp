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

using namespace std::chrono_literals;
namespace fs = std::filesystem;
namespace json = nlohmann;
namespace bk = barkeep;

static const std::vector<std::string> defaultHeaderExtensions = {".h", ".hpp", ".hh", ".hxx", ".ipp", ".tpp", ".inl"};

static const std::vector<std::string> defaultExcludeDirs = {"build", ".git", "third_party", "external"};

class FilesManager {
 public:
  FilesManager(std::vector<fs::path> paths, bool recursive = true, fs::path configPath = "toxiconf.json",
               std::vector<std::string> defaultHeaderExtensions = defaultHeaderExtensions,
               std::vector<std::string> defaultExcludeDirs = defaultExcludeDirs);
  FilesManager(fs::path configPath);
  ~FilesManager() = default;

 private:
  auto isExcludedDir(const fs::path &dirPath) -> bool;
  auto hasHeaderExtension(const fs::path &filePath) -> bool;
  auto collectPathFiles(std::vector<fs::path> paths, bool recursive) -> std::vector<fs::path>;
  auto loadConfig() -> std::expected<void, std::string>;
  auto saveConfig() -> std::expected<void, std::string>;

  std::vector<fs::path> sourcePaths_;
  fs::path configPath_;
  std::vector<std::string> excludeDirs_;
  std::vector<std::string> headerExtensions_;
};

#endif /* !FILESMANAGER_HPP_ */