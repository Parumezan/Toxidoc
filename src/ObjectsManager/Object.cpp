#include "Object.hpp"

Object::Object(const fs::path &filePath, const std::string &objName, ObjectType type, uintmax_t startLine,
               uintmax_t startColumn, uintmax_t endLine, uintmax_t endColumn, const std::string &rawComment,
               const std::string &debrief, const std::vector<std::string> &arguments,
               ObjectState state = ObjectState::Unchanged)
    : filePath_(filePath),
      name_(objName),
      type_(type),
      startLine_(startLine),
      startColumn_(startColumn),
      endLine_(endLine),
      endColumn_(endColumn),
      rawComment_(rawComment),
      debrief_(debrief),
      arguments_(arguments),
      state_(state) {}

Object::Object(const json::json &j) {
  if (j.contains("file_path") && j["file_path"].is_string()) filePath_ = fs::path(j["file_path"].get<std::string>());
  if (j.contains("name") && j["name"].is_string()) name_ = j["name"].get<std::string>();
  if (j.contains("type") && j["type"].is_string()) type_ = getObjectTypeFromString(j["type"].get<std::string>());
  if (j.contains("overload_index") && j["overload_index"].is_number_unsigned())
    overloadIndex_ = j["overload_index"].get<uintmax_t>();
  if (j.contains("start_line") && j["start_line"].is_number_unsigned()) startLine_ = j["start_line"].get<uintmax_t>();
  if (j.contains("start_column") && j["start_column"].is_number_unsigned())
    startColumn_ = j["start_column"].get<uintmax_t>();
  if (j.contains("end_line") && j["end_line"].is_number_unsigned()) endLine_ = j["end_line"].get<uintmax_t>();
  if (j.contains("end_column") && j["end_column"].is_number_unsigned()) endColumn_ = j["end_column"].get<uintmax_t>();
  if (j.contains("raw_comment") && j["raw_comment"].is_string()) rawComment_ = j["raw_comment"].get<std::string>();
  if (j.contains("debrief") && j["debrief"].is_string()) debrief_ = j["debrief"].get<std::string>();
  if (j.contains("arguments") && j["arguments"].is_array()) {
    arguments_.clear();
    for (const auto &arg : j["arguments"])
      if (arg.is_string()) arguments_.push_back(arg.get<std::string>());
  }
}

auto Object::operator==(const Object &other) const -> bool {
  return filePath_ == other.filePath_ && name_ == other.name_ && type_ == other.type_ &&
         overloadIndex_ == other.overloadIndex_;
}

auto Object::isValid() const -> bool { return !debrief_.empty(); }

auto Object::setState(ObjectState state) -> void { state_ = state; }

auto Object::setOverloadIndex(uintmax_t index) -> void { overloadIndex_ = index; }

template <typename T>
static T isModified(const T &a, const T &b, bool &modifiedFlag) {
  if (a != b) {
    modifiedFlag = true;
    return b;
  }
  return a;
}

auto Object::updateObject(const Object &other) -> void {
  bool modified = false;
  filePath_ = isModified(filePath_, other.filePath_, modified);
  name_ = isModified(name_, other.name_, modified);
  type_ = isModified(type_, other.type_, modified);
  startLine_ = other.startLine_;
  startColumn_ = other.startColumn_;
  endLine_ = other.endLine_;
  endColumn_ = other.endColumn_;
  rawComment_ = isModified(rawComment_, other.rawComment_, modified);
  debrief_ = isModified(debrief_, other.debrief_, modified);
  arguments_ = isModified(arguments_, other.arguments_, modified);
  if (modified && state_ == ObjectState::Unchanged) state_ = ObjectState::Modified;
}

auto Object::getState() const -> ObjectState { return state_; }

auto Object::getStateAsString() const -> std::string {
  switch (state_) {
    case ObjectState::Unchanged: return "Unchanged";
    case ObjectState::Modified: return "Modified";
    case ObjectState::Added: return "Added";
    case ObjectState::Removed: return "Removed";
    default: return "Unknown";
  }
}

auto Object::getObjectName() const -> std::string { return name_; }

auto Object::getObjectAsString() const -> std::string {
  std::string result;
  result +=
      "Location: " + filePath_.string() + ":" + std::to_string(startLine_) + ":" + std::to_string(startColumn_) + "\n";
  result += "Type: " + getObjectTypeAsString() + "\n";
  result += "Object Name: " + name_ + "\n";
  result += "Overload Index: " + std::to_string(overloadIndex_) + "\n";
  for (size_t i = 0; i < arguments_.size(); ++i)
    result += "Argument " + std::to_string(i) + ": " + arguments_[i] + "\n";
  result += "Raw Comment: " + rawComment_ + "\n";
  result += "Debrief: " + debrief_ + "\n";
  result += "State: " + getStateAsString() + "\n";
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
  j["overload_index"] = overloadIndex_;
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

auto Object::getObjectTypeFromString(const std::string &typeStr) -> ObjectType {
  if (typeStr == "Function")
    return ObjectType::Function;
  else if (typeStr == "Constructor")
    return ObjectType::Constructor;
  else if (typeStr == "Method")
    return ObjectType::Method;
  else if (typeStr == "Destructor")
    return ObjectType::Destructor;
  else if (typeStr == "FunctionTemplate")
    return ObjectType::FunctionTemplate;
  else if (typeStr == "Class")
    return ObjectType::Class;
  else if (typeStr == "Struct")
    return ObjectType::Struct;
  else if (typeStr == "Enum")
    return ObjectType::Enum;
  else if (typeStr == "Variable")
    return ObjectType::Variable;
  else if (typeStr == "Namespace")
    return ObjectType::Namespace;
  else if (typeStr == "Macro")
    return ObjectType::Macro;
  else
    return ObjectType::Unknown;
}