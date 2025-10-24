#ifndef OBJECTMANAGER_HPP_
#define OBJECTMANAGER_HPP_

#include <clang-c/Index.h>
#include <spdlog/spdlog.h>

#include <expected>
#include <filesystem>
#include <vector>

#include "Object.hpp"

namespace fs = std::filesystem;

class ObjectManager {
 public:
  ObjectManager() = default;
  ~ObjectManager() = default;

  auto getObjectsList() const -> const std::vector<Object> &;
  auto processHeaderFile(const fs::path &filePath) -> std::expected<void, std::string>;

 private:
  auto visitor(CXCursor cursor, CXCursor parent, CXClientData clientData) -> CXChildVisitResult;
  std::vector<Object> objects_;
  fs::path currentFilePath_;
};

#endif /* !OBJECTMANAGER_HPP_ */