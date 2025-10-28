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

  /**
   * @brief returns the list of managed objects
   *
   * @return int
   */
  auto getObjectsList() const -> const std::vector<Object> &;

  /**
   * @brief Processes a header file to extract objects
   *
   * @arg filePath
   *
   * @return std::expected<void, std::string>
   */
  auto processHeaderFile(const fs::path &filePath) -> std::expected<void, std::string>;

  /**
   * @brief Generates documentation for the managed objects
   *
   * @return void
   */
  auto generateDocumentation() -> void;

 private:
  /**
   * @brief creates documentation string for an object
   *
   * @arg obj
   * @arg columnOffset
   *
   * @return std::string
   */
  auto getDocForObject(const Object &obj, size_t columnOffset) -> std::string;

  /**
   * @brief Clang visitor function to traverse nodes
   *
   * @arg cursor
   * @arg parent
   * @arg clientData
   *
   * @return CXChildVisitResult
   */
  auto visitor(CXCursor cursor, CXCursor parent, CXClientData clientData) -> CXChildVisitResult;

  /**
   * @brief Sets overload counters for objects with the same name
   *
   * @return void
   */
  auto setOverloadCounter() -> void;

  std::vector<std::string> wordsBlacklist_;
  std::vector<std::string> typesBlacklist_;
  std::vector<Object> objects_;
  fs::path currentFilePath_;
};

#endif /* !OBJECTSMANAGER_HPP_ */