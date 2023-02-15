#pragma once

#include "bitmap.hpp"

#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <future>
#include <memory>
#include <mutex>
#include <vector>

namespace boost::iostreams
{
class mapped_file_sink;
}

class BlockData
{
public:
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
