#pragma once

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
} // namespace util
