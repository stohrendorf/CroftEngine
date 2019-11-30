#pragma once

#include "util/cimgwrapper.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/log/trivial.hpp>
#include <utility>

namespace loader::file
{
class TextureCache
{
public:
  explicit TextureCache(boost::filesystem::path basePath)
      : m_basePath{std::move(basePath)}
  {
  }

  [[nodiscard]] util::CImgWrapper loadPng(const std::string& name, uint32_t mipLevel) const
  {
    const auto path = buildPngPath(name, mipLevel);
    BOOST_LOG_TRIVIAL(debug) << "Loading " << path;
    return util::CImgWrapper{path};
  }

  [[nodiscard]] bool exists(const std::string& name, uint32_t mipLevel) const
  {
    return is_regular_file(buildPngPath(name, mipLevel));
  }

  [[nodiscard]] std::chrono::system_clock::time_point getWriteTime(const std::string& name, uint32_t mipLevel) const
  {
    return std::chrono::system_clock::from_time_t(last_write_time(buildPngPath(name, mipLevel)));
  }

  void savePng(const std::string& name, uint32_t mipLevel, util::CImgWrapper& image) const
  {
    const auto path = buildPngPath(name, mipLevel);
    BOOST_LOG_TRIVIAL(debug) << "Saving " << path;
    create_directories(path.parent_path());
    image.savePng(path);
  }

  [[nodiscard]] boost::filesystem::path buildPngPath(const std::string& name, uint32_t mipLevel) const
  {
    return m_basePath / (name + "_" + std::to_string(mipLevel) + ".png");
  }

private:
  const boost::filesystem::path m_basePath;
};
} // namespace loader
