#ifndef OBJECTSMANAGER_HPP_
#define OBJECTSMANAGER_HPP_

#include <clang-c/Index.h>
#include <spdlog/spdlog.h>

#include <expected>
#include <filesystem>
#include <vector>

#include "Object.hpp"

namespace fs = std::filesystem;

class ObjectsManager {
 public:
  ObjectsManager() = default;
  ~ObjectsManager() = default;

  auto getObjectsList() const -> const std::vector<Object> &;
  auto processHeaderFile(const fs::path &filePath) -> std::expected<void, std::string>;

 private:
  auto visitor(CXCursor cursor, CXCursor parent, CXClientData clientData) -> CXChildVisitResult;
  auto setOverloadCounter() -> void;
  std::vector<Object> objects_;
  fs::path currentFilePath_;
};

#endif /* !OBJECTSMANAGER_HPP_ */