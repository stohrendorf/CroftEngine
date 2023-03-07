#include "discfs.h"

#include "discimage.h"

#include <boost/throw_exception.hpp>
#include <cstring>
#include <filesystem>
#include <gsl/gsl-lite.hpp>
#include <map>

namespace image
{
namespace
{
uint32_t readUint32(const void* address)
{
  const auto bytes = static_cast<const uint8_t*>(address);
  uint32_t ret = bytes[0];
  ret |= bytes[1] << 8u;
  ret |= bytes[2] << 16u;
  ret |= bytes[3] << 24u;
  return ret;
}

#pragma pack(push, 1)
// ECMA 119 7.3.3
struct BothByteOrders
{
  uint32_t le = 0;
  uint32_t be = 0;
};

// ECMA 119 9.1
struct DirectoryRecord
{
  uint8_t LEN_DR = 0;
  uint8_t extendedAttributeLength = 0;
  BothByteOrders extentLocation{};
  BothByteOrders dataLength{};
  uint8_t dateTime[7]{};
  uint8_t fileFlags = 0;
  uint8_t fileUnitSize = 0;
  uint8_t interleaveGapSize = 0;
  uint32_t volumeSequenceNumber = 0;
  uint8_t LEN_FI = 0;
  char identifierStart = 0;
};
#pragma pack(pop)
} // namespace

void visitDir(DiscImage& drive,
              std::map<std::filesystem::path, FileSpan>& fileMap,
              const std::filesystem::path& parentPath,
              size_t dirSize,
              size_t sector)
{
  while(dirSize > 0)
  {
    std::vector<uint8_t> sectorData = drive.readSector(sector);
    if(sectorData.empty())
      return;

    size_t sectorOffset = 0;
    while(dirSize > 0)
    {
      gsl_Assert(sectorData.size() - sectorOffset > sizeof(DirectoryRecord));
      const auto record = (const DirectoryRecord*)&sectorData.at(sectorOffset);
      if(record->LEN_DR == 0)
      {
        return;
      }

      const auto nextAction = gsl::final_action(
        [&]()
        {
          sectorOffset += record->LEN_DR;
          if(sectorOffset + offsetof(DirectoryRecord, identifierStart) > 2048)
          {
            dirSize -= 2048;
            sector++;
          }
        });

      if(record->fileFlags & 4u)
      {
        // associated file
        continue;
      }

      gsl_Assert(offsetof(DirectoryRecord, identifierStart) + record->LEN_FI + sectorOffset < sectorData.size());
      const size_t nameLength = record->LEN_FI;
      std::string entryName{&record->identifierStart, &record->identifierStart + record->LEN_FI};

      if(nameLength == 1)
      {
        // this can only mean it's the root or parent directory
        continue;
      }

      // ECMA 119 7.5.1 <filename>.<extension>;<version>
      if(const auto nul = entryName.find_first_of('\0'); nul != std::string::npos)
        entryName = entryName.substr(0, nul);
      if(const auto separator = entryName.find_last_of(';'); separator != std::string::npos)
        entryName = entryName.substr(0, separator);

      const auto entryPath = parentPath / entryName;
      if(record->fileFlags & 2u)
      {
        // directory
        visitDir(drive, fileMap, entryPath, record->dataLength.le, record->extentLocation.le);
      }
      else
      {
        // file
        gsl_Assert(fileMap.find(entryPath) == fileMap.end());
        fileMap[entryPath] = FileSpan{record->extentLocation.le, record->dataLength.le};
      }
    }
  }
}

std::map<std::filesystem::path, FileSpan> getFiles(DiscImage& drive)
{
  std::vector<uint8_t> sectorBuffer = drive.readSector(16);
  if(sectorBuffer.empty())
    return {};

  if(std::strncmp("CD001", (const char*)&sectorBuffer[1], 5) != 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("not an ISO 9660 image"));
  }
  const size_t dirEntrySector = readUint32(&sectorBuffer[158]);
  const size_t dirSize = readUint32(&sectorBuffer[166]);

  std::map<std::filesystem::path, FileSpan> fileMap;
  visitDir(drive, fileMap, {}, dirSize, dirEntrySector);

  return fileMap;
}

std::vector<uint8_t> readFile(DiscImage& drive, const FileSpan& span)
{
  auto buffer = drive.read(span.sector, span.size);
  if(buffer.size() != gsl::narrow<size_t>(span.size))
    BOOST_THROW_EXCEPTION(std::runtime_error("could not read file"));
  return buffer;
}
} // namespace image
