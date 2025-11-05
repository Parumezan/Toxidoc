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

/**
 * @brief Allows file management and backup system management
 *
 * @class FilesManager
 */
class FilesManager {
 public:
  /**
   * @brief Constructs a FilesManager with the given parameters
   *
   * @arg configPath Path to the configuration file
   * @arg noSave If true, the configuration will not be saved after initialization
   * @arg paths List of source paths to process
   * @arg defaultHeaderExtensions List of default header file extensions
   * @arg defaultExcludeDirs List of default directories to exclude
   * @arg wordsBlacklist List of words to designate names to ignore
   * @arg typesBlacklist List of object types to ignore
   * @arg recursive If true, directories will be searched recursively
   */
  FilesManager(fs::path configPath, bool noSave, fs::path modPath, std::vector<std::string> paths,
               std::vector<std::string> defaultHeaderExtensions, std::vector<std::string> defaultExcludeDirs,
               std::vector<std::string> wordsBlacklist, std::vector<std::string> typesBlacklist, bool recursive);

  /**
   * @brief Destructor for FilesManager
   */
  ~FilesManager() = default;

  /**
   * @brief Initializes the FilesManager by loading configuration and collecting source files
   */
  auto init() -> std::expected<void, std::string>;

  /**
   * @brief Gets the module path for clang parsing
   */
  auto getModulePath() const -> fs::path;

  /**
   * @brief Gets the list of source paths
   */
  auto getSourcePaths() const -> std::vector<fs::path>;

  /**
   * @brief Gets the list of saved objects from the configuration
   */
  auto getSavedObjects() -> std::vector<Object>;

  /**
   * @brief Gets the blacklist of words we want to ignore
   */
  auto getWordsBlacklist() const -> std::vector<std::string>;

  /**
   * @brief Gets the blacklist of object types we want to ignore
   */
  auto getTypesBlacklist() const -> std::vector<std::string>;

  /**
   * @brief Gets the last save time of the configuration
   */
  auto getLastSaveTime() const -> std::chrono::system_clock::time_point;

  /**
   * @brief Saves the current configuration to the config file
   *
   * @param objects List of objects to save
   */
  auto saveConfig(std::vector<Object> objects = {}) -> std::expected<void, std::string>;

 private:
  /**
   * @brief Loads the configuration from the config file
   */
  auto loadConfig() -> std::expected<void, std::string>;

  /**
   * @brief Checks if a directory is in the exclude list
   *
   * @param dirPath Path to the directory to check
   */
  auto isExcludedDir(const fs::path &dirPath) -> bool;

  /**
   * @brief Checks if a file has a header file extension
   *
   * @param filePath Path to the file to check
   */
  auto hasHeaderExtension(const fs::path &filePath) -> bool;

  /**
   * @brief Collects all header files from the given paths
   *
   * @param paths List of paths to collect files from
   */
  auto collectPathFiles(std::vector<fs::path> paths) -> std::expected<std::vector<fs::path>, std::string>;

  bool recursive_;
  bool noSave_;
  std::vector<fs::path> sourcePaths_;
  fs::path configPath_;
  fs::path modPath_;
  std::vector<std::string> excludeDirs_;
  std::vector<std::string> headerExtensions_;
  std::vector<std::string> wordsBlacklist_;
  std::vector<std::string> typesBlacklist_;
  std::vector<Object> objects_;
  std::chrono::system_clock::time_point lastSaveTime_;
};

#endif /* !FILESMANAGER_HPP_ */