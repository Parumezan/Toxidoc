#ifndef OBJECTSMANAGER_HPP_
#define OBJECTSMANAGER_HPP_

#include <clang-c/Index.h>
#include <clang-c/Rewrite.h>
#include <spdlog/spdlog.h>

#include <expected>
#include <filesystem>
#include <fstream>
#include <vector>

#include "Object.hpp"

namespace fs = std::filesystem;

const std::map<ObjectType, std::string> ObjectTypeStringDocMap = {
    {ObjectType::Class, "@class"},
};

/**
 * @brief Manages a collection of Object instances by parsing header files
 *
 * @class ObjectsManager
 */
class ObjectsManager {
 public:
  /**
   * @brief Constructs an ObjectsManager instance
   *
   * @param blacklist List of words to ignore when processing objects
   */
  ObjectsManager(std::vector<std::string> wordsBlacklist = {}, std::vector<std::string> typesBlacklist = {});

  /**
   * @brief Destructor for ObjectsManager
   */
  ~ObjectsManager() = default;

  auto getObjectsList() const -> const std::vector<Object> &;
  auto processHeaderFile(const fs::path &filePath) -> std::expected<void, std::string>;
  auto generateDocumentation() -> std::expected<void, std::string>;

 private:
  auto getDocForObject(const Object &obj, size_t columnOffset) -> std::string;
  auto visitor(CXCursor cursor, CXCursor parent, CXClientData clientData) -> CXChildVisitResult;
  auto setOverloadCounter() -> void;

  std::vector<std::string> wordsBlacklist_;
  std::vector<std::string> typesBlacklist_;
  std::vector<Object> objects_;
  fs::path currentFilePath_;
};

#endif /* !OBJECTSMANAGER_HPP_ */