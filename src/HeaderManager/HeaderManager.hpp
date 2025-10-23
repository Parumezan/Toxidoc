#ifndef HEADERMANAGER_HPP_
#define HEADERMANAGER_HPP_

#include <clang-c/Index.h>
#include <spdlog/spdlog.h>

#include <expected>
#include <filesystem>
#include <vector>

#include "HeaderObject/HeaderObject.hpp"

namespace fs = std::filesystem;

class HeaderManager {
 public:
  HeaderManager() = default;
  ~HeaderManager() = default;
  auto getObjectsList() const -> const std::vector<HeaderObject> &;
  auto processHeaderFile(const fs::path &filePath) -> std::expected<void, std::string>;

 private:
  auto visitor(const fs::path &filePath, CXCursor cursor, CXCursor parent, CXClientData clientData)
      -> CXChildVisitResult;
  std::vector<HeaderObject> headerObjects_;
};

#endif /* !HEADERMANAGER_HPP_ */