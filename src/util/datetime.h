#pragma once

#include "core/i18n.h"

#include <boost/format.hpp>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <gsl/gsl-lite.hpp>
#include <iomanip>
#include <sstream>
#include <string>

namespace util
{
[[nodiscard]] inline ::tm toLocalTime(std::time_t time)
{
  struct tm localTimeData
  {
  };
#ifdef WIN32
  gsl_Assert(localtime_s(&localTimeData, &time) == 0);
  return localTimeData;
#else
  gsl_Assert(localtime_r(&time, &localTimeData) != nullptr);
  return localTimeData;
#endif
}

[[nodiscard]] inline std::string toHumanReadableTimestamp(std::time_t time)
{
  const auto localTime = toLocalTime(time);
  return (boost::format("%04d-%02d-%02d %02d-%02d-%02d") % (localTime.tm_year + 1900) % (localTime.tm_mon + 1)
          % localTime.tm_mday % localTime.tm_hour % localTime.tm_min % localTime.tm_sec)
    .str();
}

[[nodiscard]] inline std::string getCurrentHumanReadableTimestamp()
{
  return toHumanReadableTimestamp(std::time(nullptr));
}

[[nodiscard]] inline std::string toSavegameTime(const std::filesystem::file_time_type& saveTime,
                                                const std::string& locale)
{
  const auto timePoint
    = std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(
      saveTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()));
  const auto localTime = util::toLocalTime(timePoint);
  std::stringstream timeStr;
  try
  {
    timeStr.imbue(std::locale{locale});
  }
  catch(...)
  {
    timeStr.imbue(std::locale{});
  }
  timeStr << std::put_time(&localTime,
                           /* translators: TR charmap encoding */ pgettext("SavegameTime", "%d %B %Y %X"));
  return timeStr.str();
}
} // namespace util
