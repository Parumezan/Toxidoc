#ifndef HEADEROBJECT_HPP_
#define HEADEROBJECT_HPP_

#include <filesystem>

namespace fs = std::filesystem;

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

class HeaderObject {
 public:
  HeaderObject(ObjectType type, const fs::path &filePath, uintmax_t startLine, uintmax_t startColumn, uintmax_t endLine,
               uintmax_t endColumn, const std::string &rawComment, const std::string &debrief);
  ~HeaderObject() = default;
  auto getObjectAsString() const -> std::string;

 private:
  auto getObjectTypeAsString() const -> std::string;

  ObjectType type_;
  fs::path filePath_;
  uintmax_t startLine_;
  uintmax_t startColumn_;
  uintmax_t endLine_;
  uintmax_t endColumn_;
  std::string rawComment_;
  std::string debrief_;
};

#endif /* !HEADEROBJECT_HPP_ */