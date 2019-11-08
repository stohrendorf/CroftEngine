#pragma once

#include "util/cimgwrapper.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/log/trivial.hpp>
#include <utility>

namespace loader
{
namespace file
{
class TextureCache
{
public:
  explicit TextureCache(boost::filesystem::path basePath)
      : m_basePath{std::move(basePath)}
  {
  }

  util::CImgWrapper loadPng(const std::string& name, uint32_t mipLevel) const
  {
    const auto path = buildPngPath(name, mipLevel);
    BOOST_LOG_TRIVIAL(debug) << "Loading " << path;
    return util::CImgWrapper{path};
  }

  bool exists(const std::string& name, uint32_t mipLevel) const
  {
    return is_regular_file(buildPngPath(name, mipLevel));
  }

  std::chrono::system_clock::time_point getWriteTime(const std::string& name, uint32_t mipLevel) const
  {
    return std::chrono::system_clock::from_time_t(last_write_time(buildPngPath(name, mipLevel)));
  }

  void savePng(const std::string& name, uint32_t mipLevel, util::CImgWrapper& image)
  {
    const auto path = buildPngPath(name, mipLevel);
    BOOST_LOG_TRIVIAL(debug) << "Saving " << path;
    create_directories(path.parent_path());
    image.savePng(path);
  }

  boost::filesystem::path buildPngPath(const std::string& name, uint32_t mipLevel) const
  {
    return m_basePath / (name + "_" + std::to_string(mipLevel) + ".png");
  }

private:
  const boost::filesystem::path m_basePath;
};
} // namespace file
} // namespace loader
