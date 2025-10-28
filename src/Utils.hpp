#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <chrono>
#include <iostream>

auto cleanupProgressBar() -> void {
#if defined(_WIN32)
  std::cout << "\r";
#else
  std::cout << "\033[A\33[2K\r";
#endif
}

auto getReadableTimeString(std::chrono::_V2::system_clock::time_point timePoint) -> std::string {
  std::time_t newTimePoint = std::chrono::system_clock::to_time_t(timePoint);
  std::tm buf;
#if defined(_WIN32)
  localtime_s(&buf, &newTimePoint);
#else
  localtime_r(&newTimePoint, &buf);
#endif
  char timeStr[20];
  std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &buf);
  return std::string(timeStr);
}

#endif /* !UTILS_HPP_ */