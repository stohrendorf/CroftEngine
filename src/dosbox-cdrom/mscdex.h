#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <vector>

namespace cdrom
{
struct FileSpan
{
  size_t sector;
  std::streamsize size;
};

class CdImage;

extern std::map<std::filesystem::path, FileSpan> getFiles(CdImage& drive);
extern std::vector<uint8_t> readFile(CdImage& drive, const FileSpan& span);
} // namespace cdrom
