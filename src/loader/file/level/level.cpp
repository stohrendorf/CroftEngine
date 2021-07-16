#include "level.h"

#include "render/textureanimator.h"
#include "tr1level.h"
#include "tr2level.h"
#include "tr3level.h"
#include "tr4level.h"
#include "tr5level.h"
#include "util/md5.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <filesystem>

namespace loader::file::level
{
Level::~Level() = default;

/// \brief reads the mesh data.
void Level::readMeshData(io::SDLReader& reader)
{
  const auto meshDataWords = reader.readU32();
  const auto basePos = reader.tell();

  const auto meshDataSize = meshDataWords * 2;
  reader.skip(meshDataSize);

  std::vector<uint32_t> offsets;
  reader.readVector(offsets, reader.readU32());
  std::set<uint32_t> uniqueOffsets{offsets.begin(), offsets.end()};
  const auto endPos = reader.tell();

  m_meshes.clear();
  for(const auto offset : uniqueOffsets)
  {
    reader.seek(basePos + std::streamoff(offset));

    if(gameToEngine(m_gameVersion) >= Engine::TR4)
      m_meshes.emplace_back(*Mesh::readTr4(reader));
    else
      m_meshes.emplace_back(*Mesh::readTr1(reader));
  }
  Ensures(m_meshes.size() == uniqueOffsets.size());

  m_meshIndices.clear();
  std::transform(offsets.begin(),
                 offsets.end(),
                 std::back_inserter(m_meshIndices),
                 [&uniqueOffsets](uint32_t offset)
                 {
                   auto it = uniqueOffsets.find(offset);
                   Expects(it != uniqueOffsets.end());
                   return std::distance(uniqueOffsets.begin(), it);
                 });

  reader.seek(endPos);
}

std::unique_ptr<Level> Level::createLoader(const std::filesystem::path& filename, Game gameVersion)
{
  util::ensureFileExists(filename);

  std::filesystem::path sfxPath = filename;
  sfxPath.replace_filename("MAIN.SFX");

  io::SDLReader reader{filename};
  if(!reader.isOpen())
    return nullptr;

  if(gameVersion == Game::Unknown)
    gameVersion = probeVersion(reader, filename);
  if(gameVersion == Game::Unknown)
    return nullptr;

  reader.seek(0);
  return createLoader(std::move(reader), filename, gameVersion, sfxPath);
}

std::unique_ptr<Level> Level::createLoader(io::SDLReader&& reader,
                                           const std::filesystem::path& filename,
                                           Game game_version,
                                           const std::filesystem::path& sfxPath)
{
  if(!reader.isOpen())
    return nullptr;

  std::unique_ptr<Level> result;

  switch(game_version)
  {
  case Game::TR1: result = std::make_unique<TR1Level>(game_version, std::move(reader), filename); break;
  case Game::TR1Demo:
  case Game::TR1UnfinishedBusiness:
    result = std::make_unique<TR1Level>(game_version, std::move(reader), filename);
    result->m_demoOrUb = true;
    break;
  case Game::TR2: result = std::make_unique<TR2Level>(game_version, std::move(reader), filename); break;
  case Game::TR2Demo:
    result = std::make_unique<TR2Level>(game_version, std::move(reader), filename);
    result->m_demoOrUb = true;
    break;
  case Game::TR3: result = std::make_unique<TR3Level>(game_version, std::move(reader), filename); break;
  case Game::TR4:
  case Game::TR4Demo: result = std::make_unique<TR4Level>(game_version, std::move(reader), filename); break;
  case Game::TR5: result = std::make_unique<TR5Level>(game_version, std::move(reader), filename); break;
  default: BOOST_THROW_EXCEPTION(std::runtime_error("Invalid game version"));
  }

  result->m_sfxPath = sfxPath;
  return result;
}

Game Level::probeVersion(io::SDLReader& reader, const std::filesystem::path& filename)
{
  if(!reader.isOpen() || !std::filesystem::is_regular_file(filename))
    return Game::Unknown;

  const std::string ext = boost::algorithm::to_upper_copy(filename.extension().string());

  reader.seek(0);
  std::array<uint8_t, 4> check{};
  reader.readBytes(check.data(), check.size());

  Game ret = Game::Unknown;
  if(ext == ".PHD")
  {
    if(check[0] == 0x20 && check[1] == 0x00 && check[2] == 0x00 && check[3] == 0x00)
    {
      ret = Game::TR1;
    }
  }
  else if(ext == ".TUB")
  {
    if(check[0] == 0x20 && check[1] == 0x00 && check[2] == 0x00 && check[3] == 0x00)
    {
      ret = Game::TR1UnfinishedBusiness;
    }
  }
  else if(ext == ".TR2")
  {
    if(check[0] == 0x2D && check[1] == 0x00 && check[2] == 0x00 && check[3] == 0x00)
    {
      ret = Game::TR2;
    }
    else if((check[0] == 0x38 || check[0] == 0x34) && check[1] == 0x00 && (check[2] == 0x18 || check[2] == 0x08)
            && check[3] == 0xFF)
    {
      ret = Game::TR3;
    }
  }
  else if(ext == ".TR4")
  {
    if(check[0] == 0x54 && // T
       check[1] == 0x52 && // R
       check[2] == 0x34 && // 4
       check[3] == 0x00)
    { // NOLINT(bugprone-branch-clone)
      ret = Game::TR4;
    }
    else if(check[0] == 0x54 && // T
            check[1] == 0x52 && // R
            check[2] == 0x34 && // 4
            check[3] == 0x63)   //
    {
      ret = Game::TR4;
    }
    else if(check[0] == 0xF0 && check[1] == 0xFF && check[2] == 0xFF && check[3] == 0xFF)
    {
      ret = Game::TR4;
    }
  }
  else if(ext == ".TRC")
  {
    if(check[0] == 0x54 && // T
       check[1] == 0x52 && // R
       check[2] == 0x43 && // C
       check[3] == 0x00)
    {
      ret = Game::TR5;
    }
  }

  return ret;
}

void Level::convertTexture(ByteTexture& tex, Palette& pal, DWordTexture& dst)
{
  for(int y = 0; y < 256; y++)
  {
    for(int x = 0; x < 256; x++)
    {
      const auto col = tex.pixels[y][x];

      if(col > 0)
        dst.pixels[y][x] = {pal.colors[col].r, pal.colors[col].g, pal.colors[col].b, 255};
      else
        dst.pixels[y][x] = {0, 0, 0, 0};
    }
  }

  dst.md5 = util::md5(&tex.pixels[0][0], 256 * 256);
}

void Level::convertTexture(WordTexture& tex, DWordTexture& dst)
{
  for(int y = 0; y < 256; y++)
  {
    for(int x = 0; x < 256; x++)
    {
      const auto col = tex.pixels[y][x];

      if((col & 0x8000u) != 0)
      {
        const auto r = static_cast<uint8_t>((col & 0x00007c00u) >> 7u);
        const auto g = static_cast<uint8_t>((col & 0x000003e0u) >> 2u);
        const auto b = static_cast<uint8_t>((col & 0x0000001fu) << 3u);
        dst.pixels[y][x] = {r, g, b, 1};
      }
      else
      {
        dst.pixels[y][x] = {0, 0, 0, 0};
      }
    }
  }
}
} // namespace loader::file::level
