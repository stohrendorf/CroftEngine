#pragma once

#include <boost/log/trivial.hpp>
#include <filesystem>
#include <gl/cimgwrapper.h>
#include <utility>

namespace loader::file
{
class TextureCache
{
public:
  explicit TextureCache(std::filesystem::path basePath)
      : m_basePath{std::move(basePath)}
  {
  }

  [[nodiscard]] gl::CImgWrapper loadPng(const std::string& name, uint32_t index, uint32_t mipLevel) const
  {
    const auto path = buildPngPath(name, index, mipLevel);
    BOOST_LOG_TRIVIAL(debug) << "Loading " << path;
    return gl::CImgWrapper{path};
  }

  [[nodiscard]] bool exists(const std::string& name, uint32_t index, uint32_t mipLevel) const
  {
    return is_regular_file(buildPngPath(name, index, mipLevel));
  }

  [[nodiscard]] auto getWriteTime(const std::string& name, uint32_t index, uint32_t mipLevel) const
  {
    return last_write_time(buildPngPath(name, index, mipLevel));
  }

  void savePng(const std::string& name, uint32_t index, uint32_t mipLevel, gl::CImgWrapper& image) const
  {
    const auto path = buildPngPath(name, index, mipLevel);
    BOOST_LOG_TRIVIAL(debug) << "Saving " << path;
    create_directories(path.parent_path());
    image.savePng(path);
  }

  [[nodiscard]] std::filesystem::path buildPngPath(const std::string& name, uint32_t index, uint32_t mipLevel) const
  {
    return m_basePath / (name + "_" + std::to_string(index) + "_" + std::to_string(mipLevel) + ".png");
  }

private:
  const std::filesystem::path m_basePath;
};
} // namespace loader::file
