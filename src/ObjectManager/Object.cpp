#include "Object.hpp"

Object::Object(const fs::path &filePath, const std::string &objName, ObjectType type, uintmax_t startLine,
               uintmax_t startColumn, uintmax_t endLine, uintmax_t endColumn, const std::string &rawComment,
               const std::string &debrief, const std::vector<std::string> &arguments)
    : filePath_(filePath),
      name_(objName),
      type_(type),
      startLine_(startLine),
      startColumn_(startColumn),
      endLine_(endLine),
      endColumn_(endColumn),
      rawComment_(rawComment),
      debrief_(debrief),
      arguments_(arguments) {}

auto Object::getObjectAsString() const -> std::string {
  std::string result;
  result +=
      "Location: " + filePath_.string() + ":" + std::to_string(startLine_) + ":" + std::to_string(startColumn_) + "\n";
  result += "Type: " + getObjectTypeAsString() + "\n";
  result += "Object Name: " + name_ + "\n";
  for (size_t i = 0; i < arguments_.size(); ++i)
    result += "Argument " + std::to_string(i) + ": " + arguments_[i] + "\n";
  result += "Raw Comment: " + rawComment_ + "\n";
  result += "Debrief: " + debrief_ + "\n";
  return result;
}

auto Object::getObjectPathAsString() const -> std::string {
  std::string result;
  result += filePath_.string() + ":";
  result += std::to_string(startLine_) + ":" + std::to_string(startColumn_);
  return result;
}

auto Object::getObjectAsJSON() const -> json::json {
  json::json j;
  j["file_path"] = filePath_.string();
  j["name"] = name_;
  j["type"] = getObjectTypeAsString();
  j["start_line"] = startLine_;
  j["start_column"] = startColumn_;
  j["end_line"] = endLine_;
  j["end_column"] = endColumn_;
  j["raw_comment"] = rawComment_;
  j["debrief"] = debrief_;
  j["arguments"] = arguments_;
  return j;
}

auto Object::getObjectTypeAsString() const -> std::string {
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