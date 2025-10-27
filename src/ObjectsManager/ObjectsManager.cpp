#include "ObjectsManager.hpp"

#include <iostream>

auto ObjectsManager::getObjectsList() const -> const std::vector<Object> & { return objects_; }

auto ObjectsManager::processHeaderFile(const fs::path &filePath) -> std::expected<void, std::string> {
  CXIndex index = clang_createIndex(0, 0);
  if (!index) { return std::unexpected("Failed to create Clang index"); }

  const char *args[] = {"-std=c++23", "-I."};
  CXTranslationUnit translationUnit = clang_parseTranslationUnit(
      index, filePath.string().c_str(), args, sizeof(args) / sizeof(args[0]), nullptr, 0, CXTranslationUnit_None);
  if (!translationUnit) {
    clang_disposeIndex(index);
    return std::unexpected("Failed to parse translation unit for file: " + filePath.string());
  }

  CXCursor rootCursor = clang_getTranslationUnitCursor(translationUnit);
  currentFilePath_ = filePath;

  clang_visitChildren(
      rootCursor,
      [](CXCursor cursor, CXCursor parent, CXClientData clientData) {
        ObjectsManager *manager = static_cast<ObjectsManager *>(clientData);
        return manager->visitor(cursor, parent, clientData);
      },
      this);

  clang_disposeTranslationUnit(translationUnit);
  clang_disposeIndex(index);
  setOverloadCounter();
  return {};
}

auto ObjectsManager::visitor(CXCursor cursor, CXCursor parent, CXClientData clientData) -> CXChildVisitResult {
  CXSourceLocation loc = clang_getCursorLocation(cursor);
  if (!clang_Location_isFromMainFile(loc)) return CXChildVisit_Continue;

  CXFile cxFile = nullptr;
  unsigned line = 0, column = 0, offset = 0;
  clang_getSpellingLocation(loc, &cxFile, &line, &column, &offset);

  if (!cxFile) return CXChildVisit_Continue;

  CXString cxFileName = clang_getFileName(cxFile);
  const char *cFileName = clang_getCString(cxFileName);
  std::string foundPath = cFileName ? cFileName : "";
  clang_disposeString(cxFileName);

  std::error_code ec;
  fs::path asked = fs::weakly_canonical(currentFilePath_, ec);
  fs::path found = fs::weakly_canonical(foundPath, ec);
  if (asked != found) return CXChildVisit_Continue;

  CXCursorKind kind = clang_getCursorKind(cursor);
  ObjectType objType = ObjectType::Unknown;

  switch (kind) {
    case CXCursor_FunctionDecl: objType = ObjectType::Function; break;
    case CXCursor_CXXMethod: objType = ObjectType::Method; break;
    case CXCursor_Constructor: objType = ObjectType::Constructor; break;
    case CXCursor_Destructor: objType = ObjectType::Destructor; break;
    case CXCursor_FunctionTemplate: objType = ObjectType::FunctionTemplate; break;
    case CXCursor_ClassDecl: objType = ObjectType::Class; break;
    case CXCursor_StructDecl: objType = ObjectType::Struct; break;
    case CXCursor_EnumDecl: objType = ObjectType::Enum; break;
    case CXCursor_VarDecl: objType = ObjectType::Variable; break;
    case CXCursor_Namespace: objType = ObjectType::Namespace; break;
    case CXCursor_MacroDefinition: objType = ObjectType::Macro; break;
    default: objType = ObjectType::Unknown; break;
  }

  if (objType != ObjectType::Unknown) {
    CXSourceRange sourceRange = clang_getCursorExtent(cursor);
    CXSourceLocation startLocation = clang_getRangeStart(sourceRange);
    CXSourceLocation endLocation = clang_getRangeEnd(sourceRange);

    unsigned startLine, startColumn, endLine, endColumn;
    clang_getSpellingLocation(startLocation, nullptr, &startLine, &startColumn, nullptr);
    clang_getSpellingLocation(endLocation, nullptr, &endLine, &endColumn, nullptr);

    CXString rawCommentCX = clang_Cursor_getRawCommentText(cursor);
    const char *rawCommentCStr = clang_getCString(rawCommentCX);
    std::string rawComment = rawCommentCStr ? rawCommentCStr : "";
    clang_disposeString(rawCommentCX);

    CXString debriefCX = clang_Cursor_getBriefCommentText(cursor);
    const char *debriefCStr = clang_getCString(debriefCX);
    std::string debrief = debriefCStr ? debriefCStr : "";
    clang_disposeString(debriefCX);

    CXString nameCX = clang_getCursorSpelling(cursor);
    const char *nameCStr = clang_getCString(nameCX);
    std::string objectName = nameCStr ? nameCStr : "";
    clang_disposeString(nameCX);

    std::vector<std::string> arguments;
    if (objType == ObjectType::Function || objType == ObjectType::Method || objType == ObjectType::Constructor ||
        objType == ObjectType::Destructor || objType == ObjectType::FunctionTemplate) {
      int numArgs = clang_Cursor_getNumArguments(cursor);
      for (int i = 0; i < numArgs; ++i) {
        CXCursor argCursor = clang_Cursor_getArgument(cursor, i);
        CXString argNameCX = clang_getCursorSpelling(argCursor);
        const char *argNameCStr = clang_getCString(argNameCX);
        std::string argName = argNameCStr ? argNameCStr : "";
        clang_disposeString(argNameCX);
        arguments.push_back(argName);
      }
    }

    CXType returnTypeCX = clang_getCursorResultType(cursor);
    CXString returnTypeStrCX = clang_getTypeSpelling(returnTypeCX);
    const char *returnTypeCStr = clang_getCString(returnTypeStrCX);
    std::string returnType = returnTypeCStr ? returnTypeCStr : "";
    clang_disposeString(returnTypeStrCX);

    Object object(currentFilePath_, objectName, objType, startLine, startColumn, endLine, endColumn, rawComment,
                  debrief, arguments, returnType, ObjectState::Unchanged);
    objects_.push_back(object);
  }
  return CXChildVisit_Recurse;
}

auto ObjectsManager::setOverloadCounter() -> void {
  std::map<std::string, size_t> overloadCounters;
  for (const auto &obj : objects_) {
    std::string key = obj.getObjectName();
    overloadCounters[key] = 0;
  }

  size_t overload = 0;
  for (auto &obj : objects_) {
    std::string key = obj.getObjectName();
    if (overloadCounters.find(key) != overloadCounters.end()) {
      overloadCounters[key]++;
      overload = overloadCounters[key];
    } else {
      overloadCounters[key] = 0;
      overload = 0;
    }
    obj.setOverloadIndex(overload);
  }
}