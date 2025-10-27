#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
namespace json = nlohmann;

enum class ObjectType {
  Unknown,
  Function,
  Constructor,
  Method,
  Destructor,
  FunctionTemplate,
  Class,
  Struct,
  Enum,
  Variable,
  Namespace,
  Macro
};

enum class ObjectState { Unchanged, Modified, Added, Removed };

class Object {
 public:
  Object(const fs::path &filePath, const std::string &objName, ObjectType type, size_t startLine, size_t startColumn,
         size_t endLine, size_t endColumn, const std::string &rawComment, const std::string &debrief,
         const std::vector<std::string> &arguments, const std::string &returnType, ObjectState state);
  Object(const json::json &j);
  ~Object() = default;
  auto operator==(const Object &other) const -> bool;
  auto isValid() const -> bool;
  auto setState(ObjectState state) -> void;
  auto setOverloadIndex(size_t index) -> void;
  auto updateObject(const Object &other) -> void;
  auto getState() const -> ObjectState;
  auto getStateAsString() const -> std::string;
  auto getObjectName() const -> std::string;
  auto getObjectAsString() const -> std::string;
  auto getObjectPathAsString() const -> std::string;
  auto getObjectAsJSON() const -> json::json;
  auto getObjectTypeAsString() const -> std::string;

 private:
  auto getObjectTypeFromString(const std::string &typeStr) -> ObjectType;

  fs::path filePath_;
  std::string name_;
  ObjectType type_;
  size_t overloadIndex_;
  size_t startLine_;
  size_t startColumn_;
  size_t endLine_;
  size_t endColumn_;
  std::string rawComment_;
  std::string debrief_;
  std::vector<std::string> arguments_;
  std::string returnType_;
  ObjectState state_;
};

#endif /* !OBJECT_HPP_ */