#pragma once

#include <boost/iostreams/device/mapped_file.hpp>
#include <cstdint>
#include <cstdio>
#include <glm/vec2.hpp>
#include <memory>

class Bitmap;

class BlockData
{
public:
  BlockData(const BlockData&) = delete;
  BlockData(BlockData&&) = delete;
  void operator=(const BlockData&) = delete;
  void operator=(BlockData&&) = delete;

  explicit BlockData(const char* fn);
  BlockData(const char* fn, const glm::ivec2& size);
  explicit BlockData(const glm::ivec2& size);
  ~BlockData();

  std::shared_ptr<Bitmap> decode();

  void processRgba(const uint32_t* src, uint32_t blocks, size_t offset, size_t width, bool useHeuristics);

private:
  uint8_t* m_data;
  glm::ivec2 m_size{};
  size_t m_dataOffset = 0;
  size_t m_maplen;
  std::unique_ptr<boost::iostreams::mapped_file_sink> m_file{nullptr};
};
