#ifndef HEADEROBJECT_HPP_
#define HEADEROBJECT_HPP_

#include <filesystem>

namespace fs = std::filesystem;

enum class ObjectType {
  Unknown,
  Function,
  Method,
  Constructor,
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
  HeaderObject() = default;
  ~HeaderObject() = default;

 private:
  ObjectType type_;
  fs::path filePath_;
  uintmax_t startLine_;
  uintmax_t startColumn_;
  uintmax_t endLine_;
  uintmax_t endColumn_;
};

#endif /* !HEADEROBJECT_HPP_ */