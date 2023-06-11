#pragma once

#include <boost/log/trivial.hpp>
#include <boost/predef/os.h>
#include <filesystem>

#if BOOST_OS_WINDOWS
#  include <boost/algorithm/string/case_conv.hpp>
#endif

namespace util
{
inline bool preferredEqual(std::filesystem::path a, std::filesystem::path b)
{
  // TODO this assumes that only windows has case-insensitive path names
#if BOOST_OS_WINDOWS
  return boost::algorithm::to_lower_copy(a.make_preferred().string())
         == boost::algorithm::to_lower_copy(b.make_preferred().string());
#else
  return a.make_preferred() == b.make_preferred();
#endif
}

inline void rename(const std::filesystem::path& srcPath, const std::filesystem::path& dstPath)
{
  BOOST_LOG_TRIVIAL(info) << "Rename " << srcPath << " to " << dstPath;
  std::filesystem::rename(srcPath, dstPath);
}
} // namespace util
