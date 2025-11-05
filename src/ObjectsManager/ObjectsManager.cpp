#include "ObjectsManager.hpp"

#include <iostream>

// "-include",
// "/home/pibe/Projects/Toxidoc/mods/clang_qt_override.h",

ObjectsManager::ObjectsManager(std::vector<std::string> wordsBlacklist, std::vector<std::string> typesBlacklist,
                               fs::path modPath)
    : objects_({}),
      wordsBlacklist_(wordsBlacklist),
      typesBlacklist_(typesBlacklist),
      currentFilePath_(""),
      modPath_(modPath),
      moduleType_(ModuleType::None) {}

auto ObjectsManager::getObjectsList() const -> const std::vector<Object> & { return objects_; }

auto ObjectsManager::processHeaderFile(const fs::path &filePath) -> std::expected<void, std::string> {
  CXIndex index = clang_createIndex(0, 0);
  if (!index) { return std::unexpected("Failed to create Clang index"); }

  std::vector<std::string> argsVec = {"-std=c++23", "-I."};
  std::vector<const char *> args;
  std::string modPathStr;
  if (!modPath_.empty() && fs::exists(modPath_)) {
    for (const auto &[modType, modName] : ModulesList) {
      if (modPath_.filename().string() == modName) {
        moduleType_ = modType;
        argsVec.push_back("-include");
        modPathStr = modPath_.string();
        argsVec.push_back(modPathStr);
        break;
      }
    }
  }
  for (const auto &arg : argsVec) args.push_back(arg.c_str());

  CXTranslationUnit translationUnit = clang_parseTranslationUnit(index, filePath.string().c_str(), args.data(),
                                                                 args.size(), nullptr, 0, CXTranslationUnit_None);
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

auto ObjectsManager::generateDocumentation() -> void {
  std::map<std::string, std::vector<Object>> docsByFile;
  for (const auto &obj : objects_) docsByFile[obj.getObjectPath().string()].push_back(obj);
  size_t objectsProcessed = 0;
  for (const auto &[filePath, objs] : docsByFile) {
    spdlog::info("Processing file: {}", filePath);

    CXIndex index = clang_createIndex(0, 0);
    if (!index) {
      spdlog::error("Failed to create Clang index for file: {}", filePath);
      continue;
    }
    const char *args[] = {"-std=c++23", "-I."};
    CXTranslationUnit translationUnit = clang_parseTranslationUnit(
        index, filePath.c_str(), args, sizeof(args) / sizeof(args[0]), nullptr, 0, CXTranslationUnit_None);
    if (!translationUnit) {
      spdlog::error("Failed to parse translation unit for file: {}", filePath);
      clang_disposeIndex(index);
      continue;
    }
    CXRewriter rewriter = clang_CXRewriter_create(translationUnit);
    if (!rewriter) {
      spdlog::error("Failed to create Clang rewriter for file: {}", filePath);
      clang_disposeTranslationUnit(translationUnit);
      clang_disposeIndex(index);
      continue;
    }

    for (const auto &obj : objs) {
      json::json objJson = obj.getObjectAsJSON();
      if (obj.isValid() || !objJson["raw_comment"].get<std::string>().empty()) continue;
      size_t insertLine = objJson["start_line"].get<size_t>();
      size_t columnOffset = objJson["start_column"].get<size_t>();
      std::string docString = getDocForObject(obj, columnOffset);

      CXSourceLocation insertLocation = clang_getLocation(
          translationUnit, clang_getFile(translationUnit, filePath.c_str()), insertLine, columnOffset);
      clang_CXRewriter_insertTextBefore(rewriter, insertLocation, docString.c_str());
      clang_CXRewriter_overwriteChangedFiles(rewriter);
      objectsProcessed++;
    }

    clang_CXRewriter_dispose(rewriter);
    clang_disposeTranslationUnit(translationUnit);
    clang_disposeIndex(index);
  }
  spdlog::info("Total objects processed: {}", objectsProcessed);
}

auto ObjectsManager::getDocForObject(const Object &obj, size_t columnOffset) -> std::string {
  json::json objJson = obj.getObjectAsJSON();
  std::string doc = "\n";

  auto idt = [columnOffset]() { return std::string(columnOffset - 1, ' '); };
  doc += idt() + "/**\n";
  doc += idt() + " * @brief\n";

  bool hasType = false;
  for (const auto &[type, docTag] : ObjectTypeStringDocMap) {
    if (obj.getObjectType() == type) {
      hasType = true;
      doc += idt() + " *\n";
      doc += idt() + " * " + docTag + " " + obj.getObjectName() + "\n";
      break;
    }
  }
  if (objJson.contains("arguments") && !objJson["arguments"].empty()) {
    doc += idt() + " *\n";
    for (const auto &arg : objJson["arguments"]) doc += idt() + " * @arg " + arg.get<std::string>() + "\n";
  }
  if (objJson.contains("return_type") && !objJson["return_type"].get<std::string>().empty()) {
    doc += idt() + " *\n";
    if (!objJson["return_type"].get<std::string>().empty())
      doc += idt() + " * @return " + objJson["return_type"].get<std::string>() + "\n";
  }
  doc += idt() + " */\n";
  doc += idt();
  return doc;
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

  if (moduleType_ == ModuleType::QtOverride || kind == CXCursor_CXXMethod || kind == CXCursor_FunctionDecl) {
    if (clang_Cursor_hasAttrs(cursor)) {
      clang_visitChildren(
          cursor,
          [](CXCursor child, CXCursor /*parent*/, CXClientData data) {
            if (clang_getCursorKind(child) == CXCursor_AnnotateAttr) {
              CXString annotation = clang_getCursorSpelling(child);
              const char *annotationStr = clang_getCString(annotation);
              std::string annotationString = annotationStr ? annotationStr : "";

              if (annotationStr) {
                ObjectType *objTypePtr = static_cast<ObjectType *>(data);
                std::string annot(annotationStr);
                if (annot == "qt_invokable") { *objTypePtr = ObjectType::Method; }
                // else if (annot == "qt_signal") {
                //   *objTypePtr = ObjectType::Unknown;
                // } else if (annot == "qt_slot") {
                //   *objTypePtr = ObjectType::Unknown;
                // }
              }

              clang_disposeString(annotation);
            }
            return CXChildVisit_Continue;
          },
          &objType);
    }
  }

  if (objType == ObjectType::Unknown) {
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
  }

  for (const auto &typeStr : typesBlacklist_)
    if (ObjectTypeStringMap.at(objType) == typeStr) return CXChildVisit_Continue;

  if (objType != ObjectType::Unknown) {
    CXSourceRange sourceRange = clang_getCursorExtent(cursor);
    CXSourceLocation startLocation = clang_getRangeStart(sourceRange);
    CXSourceLocation endLocation = clang_getRangeEnd(sourceRange);

    unsigned startLine, startColumn, endLine, endColumn;
    clang_getSpellingLocation(startLocation, nullptr, &startLine, &startColumn, nullptr);
    clang_getSpellingLocation(endLocation, nullptr, &endLine, &endColumn, nullptr);

    CXString nameCX = clang_getCursorSpelling(cursor);
    const char *nameCStr = clang_getCString(nameCX);
    std::string objectName = nameCStr ? nameCStr : "";
    clang_disposeString(nameCX);

    for (const auto &word : wordsBlacklist_)
      if (objectName.contains(word)) return CXChildVisit_Continue;

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

    CXString rawCommentCX = clang_Cursor_getRawCommentText(cursor);
    const char *rawCommentCStr = clang_getCString(rawCommentCX);
    std::string rawComment = rawCommentCStr ? rawCommentCStr : "";
    clang_disposeString(rawCommentCX);

    CXString debriefCX = clang_Cursor_getBriefCommentText(cursor);
    const char *debriefCStr = clang_getCString(debriefCX);
    std::string debrief = debriefCStr ? debriefCStr : "";
    clang_disposeString(debriefCX);

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