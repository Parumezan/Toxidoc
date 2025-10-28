#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
namespace json = nlohmann;

/**
 * @brief Type of the object
 */
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

/**
 * @brief Mapping of ObjectType to string representations
 */
const std::map<ObjectType, std::string> ObjectTypeStringMap = {
    {ObjectType::Unknown, "Unknown"},
    {ObjectType::Function, "Function"},
    {ObjectType::Constructor, "Constructor"},
    {ObjectType::Method, "Method"},
    {ObjectType::Destructor, "Destructor"},
    {ObjectType::FunctionTemplate, "FunctionTemplate"},
    {ObjectType::Class, "Class"},
    {ObjectType::Struct, "Struct"},
    {ObjectType::Enum, "Enum"},
    {ObjectType::Variable, "Variable"},
    {ObjectType::Namespace, "Namespace"},
    {ObjectType::Macro, "Macro"},
};

/**
 * @brief State of the object
 */
enum class ObjectState { Unchanged, Modified, Added, Removed };

/**
 * @brief Represents a code object extracted from source files
 *
 * @class Object
 */
class Object {
 public:
  /**
   * @brief Constructor for Object
   *
   * @arg filePath
   * @arg objName
   * @arg type
   * @arg startLine
   * @arg startColumn
   * @arg endLine
   * @arg endColumn
   * @arg rawComment
   * @arg debrief
   * @arg arguments
   * @arg returnType
   * @arg state
   *
   * @return void
   */
  Object(const fs::path &filePath, const std::string &objName, ObjectType type, size_t startLine, size_t startColumn,
         size_t endLine, size_t endColumn, const std::string &rawComment, const std::string &debrief,
         const std::vector<std::string> &arguments, const std::string &returnType, ObjectState state);

  /**
   * @brief Constructor for Object from JSON
   *
   * @arg j
   *
   * @return void
   */
  Object(const json::json &j);

  /**
   * @brief Destructor for Object
   *
   * @return void
   */
  ~Object() = default;

  /**
   * @brief Comparison operator
   *
   * @arg other
   *
   * @return bool
   */
  auto operator==(const Object &other) const -> bool;

  /**
   * @brief Validity check
   *
   * @return bool
   */
  auto isValid() const -> bool;

  /**
   * @brief sets the state of the object
   *
   * @arg state
   *
   * @return void
   */
  auto setState(ObjectState state) -> void;

  /**
   * @brief sets the overload index of the object
   *
   * @arg index
   *
   * @return void
   */
  auto setOverloadIndex(size_t index) -> void;

  /**
   * @brief updates the object with another object's data
   *
   * @arg other
   *
   * @return void
   */
  auto updateObject(const Object &other) -> void;

  /**
   * @brief gets the state of the object
   *
   * @return ObjectState
   */
  auto getState() const -> ObjectState;

  /**
   * @brief gets the state of the object as a string
   *
   * @return std::string
   */
  auto getStateAsString() const -> std::string;

  /**
   * @brief gets the name of the object
   *
   * @return std::string
   */
  auto getObjectName() const -> std::string;

  /**
   * @brief gets the object as a string
   *
   * @return std::string
   */
  auto getObjectAsString() const -> std::string;

  /**
   * @brief gets the path of the object
   *
   * @return fs::path
   */
  auto getObjectPath() const -> fs::path;

  /**
   * @brief gets the path of the object as a string
   *
   * @return std::string
   */
  auto getObjectPathAsString() const -> std::string;

  /**
   * @brief gets the object as a JSON representation
   *
   * @return int
   */
  auto getObjectAsJSON() const -> json::json;

  /**
   * @brief gets the type of the object
   *
   * @return ObjectType
   */
  auto getObjectType() const -> ObjectType;

  /**
   * @brief gets the type of the object as a string
   *
   * @return std::string
   */
  auto getObjectTypeAsString() const -> std::string;

 private:
  /**
   * @brief gets the ObjectType from a string
   *
   * @arg typeStr
   *
   * @return ObjectType
   */
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