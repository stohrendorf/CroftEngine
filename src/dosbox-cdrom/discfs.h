#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <ios>
#include <map>
#include <vector>

namespace image
{
class DiscImage;

struct FileSpan
{
  size_t sector;
  std::streamsize size;
};

extern std::map<std::filesystem::path, FileSpan> getFiles(DiscImage& drive);
extern std::vector<uint8_t> readFile(DiscImage& drive, const FileSpan& span);
} // namespace image
