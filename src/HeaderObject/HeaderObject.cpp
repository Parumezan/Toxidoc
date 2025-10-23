#include "HeaderObject.hpp"

HeaderObject::HeaderObject(ObjectType type, const fs::path &filePath, uintmax_t startLine, uintmax_t startColumn,
                           uintmax_t endLine, uintmax_t endColumn, const std::string &rawComment,
                           const std::string &debrief)
    : type_(type),
      filePath_(filePath),
      startLine_(startLine),
      startColumn_(startColumn),
      endLine_(endLine),
      endColumn_(endColumn),
      rawComment_(rawComment),
      debrief_(debrief) {}

auto HeaderObject::getObjectAsString() const -> std::string {
  std::string result;
  result += "Object Type: " + getObjectTypeAsString() + "\n";
  result += "File Path: " + filePath_.string() + "\n";
  result += "Start: Line " + std::to_string(startLine_) + ", Column " + std::to_string(startColumn_) + "\n";
  result += "End: Line " + std::to_string(endLine_) + ", Column " + std::to_string(endColumn_) + "\n";
  result += "Raw Comment: " + rawComment_ + "\n";
  result += "Debrief: " + debrief_ + "\n";
  return result;
}

auto HeaderObject::getObjectTypeAsString() const -> std::string {
  switch (type_) {
    case ObjectType::Unknown: return "Unknown";
    case ObjectType::Function: return "Function";
    case ObjectType::Constructor: return "Constructor";
    case ObjectType::Method: return "Method";
    case ObjectType::Destructor: return "Destructor";
    case ObjectType::FunctionTemplate: return "FunctionTemplate";
    case ObjectType::Class: return "Class";
    case ObjectType::Struct: return "Struct";
    case ObjectType::Enum: return "Enum";
    case ObjectType::Variable: return "Variable";
    case ObjectType::Namespace: return "Namespace";
    case ObjectType::Macro: return "Macro";
    default: return "Unknown";
  }
}