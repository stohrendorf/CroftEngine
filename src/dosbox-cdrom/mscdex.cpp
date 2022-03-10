#include "mscdex.h"

#include "cdrom.h"

#include <array>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <filesystem>
#include <map>
#include <queue>
#include <stack>

namespace cdrom
{
uint32_t mem_readd(const void* address)
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

void iterateDir(CdImage& drive,
                std::map<std::filesystem::path, FileSpan>& fileMap,
                const std::filesystem::path& parentPath,
                size_t dirSize,
                size_t sector,
                bool iso)
{
  while(dirSize > 0)
  {
    std::vector<uint8_t> sectorData;
    if(!drive.readSectors(sectorData, sector, 1))
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

      if((iso ? record->fileFlags : record->dateTime[6]) & 4)
      {
        continue;
      }

      gsl_Assert(offsetof(DirectoryRecord, identifierStart) + record->LEN_FI + sectorOffset < sectorData.size());
      size_t nameLength = record->LEN_FI;
      std::string entryName{&record->identifierStart, &record->identifierStart + record->LEN_FI};

      if(nameLength == 1)
      {
        if(entryName[0] == '\0') // root directory
          entryName = ".";
        else if(entryName[0] == '\1') // parent directory
          entryName = "..";
        continue;
      }

      // ECMA 119 7.5.1 <filename>.<extension>;<version>
      if(const auto nul = entryName.find_first_of('\0'); nul != std::string::npos)
        entryName = entryName.substr(0, nul);
      if(const auto separator = entryName.find_last_of(';'); separator != std::string::npos)
        entryName = entryName.substr(0, separator);

      const auto entryPath = parentPath / entryName;
      if((iso ? record->fileFlags : record->dateTime[6]) & 2)
      {
        iterateDir(drive, fileMap, entryPath, record->dataLength.le, record->extentLocation.le, iso);
      }
      else
      {
        gsl_Assert(fileMap.find(entryPath) == fileMap.end());
        fileMap[entryPath] = FileSpan{record->extentLocation.le, record->dataLength.le};
      }
    }
  }
}

std::map<std::filesystem::path, FileSpan> getFiles(CdImage& drive)
{
  std::vector<uint8_t> sectorBuffer;
  if(!drive.readSectors(sectorBuffer, 16, 1))
    return {};

  bool iso = (std::strncmp("CD001", (const char*)&sectorBuffer[1], 5) == 0);
  if(!iso && std::strncmp("CDROM", (const char*)&sectorBuffer[9], 5) != 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("not an ISO 9660 or HSF CD"));
  }
  uint16_t offset = iso ? 156 : 180;
  size_t dirEntrySector = mem_readd(&sectorBuffer[offset + 2]);
  size_t dirSize = mem_readd(&sectorBuffer[offset + 10]);

  std::map<std::filesystem::path, FileSpan> fileMap;
  iterateDir(drive, fileMap, {}, dirSize, dirEntrySector, iso);

  return fileMap;
}

std::vector<uint8_t> readFile(CdImage& drive, const FileSpan& span)
{
  std::vector<uint8_t> buffer;
  if(!drive.read(buffer, span.sector, span.size))
    BOOST_THROW_EXCEPTION(std::runtime_error("could not read file"));
  return buffer;
}
} // namespace cdrom
