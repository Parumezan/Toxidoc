#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <chrono>
#include <iostream>

/**
 * @brief Cleans up the progress bar from the console because barkeep put a \n at the end
 *
 * @return void
 */
auto cleanupProgressBar() -> void {
#if defined(_WIN32)
  std::cout << "\r";
#else
  std::cout << "\033[A\33[2K\r";
#endif
}

/**
 * @brief returns a readable time string from a time point
 *
 * @arg timePoint
 *
 * @return std::string
 */
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