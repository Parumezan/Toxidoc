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

class Object {
 public:
  Object(const fs::path &filePath, const std::string &objName, ObjectType type, uintmax_t startLine,
         uintmax_t startColumn, uintmax_t endLine, uintmax_t endColumn, const std::string &rawComment,
         const std::string &debrief, const std::vector<std::string> &arguments);
  ~Object() = default;
  auto getObjectAsString() const -> std::string;
  auto getObjectPathAsString() const -> std::string;
  auto getObjectAsJSON() const -> json::json;

 private:
  auto getObjectTypeAsString() const -> std::string;

  fs::path filePath_;
  std::string name_;
  ObjectType type_;
  uintmax_t startLine_;
  uintmax_t startColumn_;
  uintmax_t endLine_;
  uintmax_t endColumn_;
  std::string rawComment_;
  std::string debrief_;
  std::vector<std::string> arguments_;
};

#endif /* !OBJECT_HPP_ */