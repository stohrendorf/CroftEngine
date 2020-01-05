#include "texture.h"

#include "engine/objects/object.h"
#include "io/sdlreader.h"
#include "loader/file/texturecache.h"
#include "loader/trx/trx.h"
#include "util/cimgwrapper.h"

#include <boost/range/adaptor/indexed.hpp>

namespace loader::file
{
void DWordTexture::toImage(const trx::Glidos* glidos, const std::function<void(const std::string&)>& statusCallback)
{
  if(glidos == nullptr)
  {
    image = std::make_shared<render::gl::Image<render::gl::SRGBA8>>(glm::ivec2{256, 256}, &pixels[0][0]);
    return;
  }

  BOOST_LOG_TRIVIAL(info) << "Upgrading texture " << md5 << "...";

  constexpr int Scale = trx::Glidos::Resolution / 256;

  auto mapping = glidos->getMappingsForTexture(md5);
  auto cache = loader::file::TextureCache{mapping.baseDir / "_edisonengine"};

  if(cache.exists(md5, 0) && cache.getWriteTime(md5, 0) > mapping.newestSource)
  {
    statusCallback("Loading cached texture...");
    BOOST_LOG_TRIVIAL(info) << "Loading cached texture " << cache.buildPngPath(md5, 0) << "...";
    util::CImgWrapper cacheImage{cache.loadPng(md5, 0)};

    cacheImage.interleave();
    image = std::make_shared<render::gl::Image<render::gl::SRGBA8>>(
      glm::ivec2{cacheImage.width(), cacheImage.height()},
      reinterpret_cast<const render::gl::SRGBA8*>(cacheImage.data()));
    return;
  }

  statusCallback("Upgrading texture (upscaling)");
  util::CImgWrapper original{pixels[0][0].channels.data(), 256, 256, false};
  original.deinterleave();
  original.resize(trx::Glidos::Resolution, trx::Glidos::Resolution);

  for(const auto& indexedTile : mapping.tiles | boost::adaptors::indexed(0))
  {
    const auto& tile = indexedTile.value();
    statusCallback("Upgrading texture (" + std::to_string(indexedTile.index() * 100 / mapping.tiles.size()) + "%)");

    BOOST_LOG_TRIVIAL(info) << "  - Loading " << tile.second << " into " << tile.first;
    if(!is_regular_file(tile.second))
    {
      BOOST_LOG_TRIVIAL(warning) << "    File not found";
      continue;
    }

    util::CImgWrapper srcImage{tile.second.string()};
    srcImage.resize(tile.first.getWidth() * Scale, tile.first.getHeight() * Scale);
    const auto x0 = tile.first.getX0() * Scale;
    const auto y0 = tile.first.getY0() * Scale;
    for(int x = 0; x < srcImage.width(); ++x)
    {
      for(int y = 0; y < srcImage.height(); ++y)
      {
        BOOST_ASSERT(x + static_cast<int>(x0) < original.width());
        BOOST_ASSERT(y + static_cast<int>(y0) < original.height());

        for(int c = 0; c < 4; ++c)
        {
          original(x + x0, y + y0, c) = srcImage(x, y, c);
        }
      }
    }
  }

  statusCallback("Saving texture to cache...");
  BOOST_LOG_TRIVIAL(info) << "Writing texture cache " << cache.buildPngPath(md5, 0) << "...";
  cache.savePng(md5, 0, original);

  original.interleave();
  image = std::make_shared<render::gl::Image<render::gl::SRGBA8>>(
    glm::ivec2{trx::Glidos::Resolution, trx::Glidos::Resolution},
    reinterpret_cast<const render::gl::SRGBA8*>(original.data()));
}

std::unique_ptr<DWordTexture> DWordTexture::read(io::SDLReader& reader)
{
  std::unique_ptr<DWordTexture> texture{std::make_unique<DWordTexture>()};

  for(auto& row : texture->pixels)
  {
    for(auto& element : row)
    {
      const auto tmp = reader.readU32(); // format is ARGB
      const uint8_t a = (tmp >> 24) & 0xff;
      const uint8_t r = (tmp >> 16) & 0xff;
      const uint8_t g = (tmp >> 8) & 0xff;
      const uint8_t b = (tmp >> 0) & 0xff;
      element = {r, g, b, a};
    }
  }

  return texture;
}

std::unique_ptr<ByteTexture> ByteTexture::read(io::SDLReader& reader)
{
  std::unique_ptr<ByteTexture> textile{new ByteTexture()};
  reader.readBytes(reinterpret_cast<uint8_t*>(textile->pixels), 256 * 256);
  return textile;
}

std::unique_ptr<WordTexture> WordTexture::read(io::SDLReader& reader)
{
  std::unique_ptr<WordTexture> texture{new WordTexture()};

  for(auto& row : texture->pixels)
  {
    for(auto& element : row)
    {
      element = reader.readU16();
    }
  }

  return texture;
}

UVCoordinates UVCoordinates::readTr1(io::SDLReader& reader)
{
  UVCoordinates uv;
  uv.xcoordinate = reader.readI8();
  uv.xpixel = reader.readU8();
  uv.ycoordinate = reader.readI8();
  uv.ypixel = reader.readU8();
  return uv;
}

UVCoordinates UVCoordinates::readTr4(io::SDLReader& reader)
{
  UVCoordinates uv;
  uv.xcoordinate = reader.readI8();
  uv.xpixel = reader.readU8();
  uv.ycoordinate = reader.readI8();
  uv.ypixel = reader.readU8();
  if(uv.xcoordinate == 0)
  {
    uv.xcoordinate = 1;
  }
  if(uv.ycoordinate == 0)
  {
    uv.ycoordinate = 1;
  }
  return uv;
}

std::unique_ptr<TextureTile> TextureTile::readTr1(io::SDLReader& reader)
{
  std::unique_ptr<TextureTile> tile{std::make_unique<TextureTile>()};
  tile->textureKey.blendingMode = static_cast<BlendingMode>(reader.readU16());
  tile->textureKey.tileAndFlag = reader.readU16();
  if(tile->textureKey.tileAndFlag > 64)
    BOOST_LOG_TRIVIAL(warning) << "TR1 Object Texture: tileAndFlag > 64";

  if((tile->textureKey.tileAndFlag & (1 << 15)) != 0)
    BOOST_LOG_TRIVIAL(warning) << "TR1 Object Texture: tileAndFlag is flagged";

  // only in TR4
  tile->textureKey.flags = 0;
  tile->uvCoordinates[0] = UVCoordinates::readTr1(reader);
  tile->uvCoordinates[1] = UVCoordinates::readTr1(reader);
  tile->uvCoordinates[2] = UVCoordinates::readTr1(reader);
  tile->uvCoordinates[3] = UVCoordinates::readTr1(reader);
  // only in TR4
  tile->unknown1 = 0;
  tile->unknown2 = 0;
  tile->x_size = 0;
  tile->y_size = 0;
  return tile;
}

std::unique_ptr<TextureTile> TextureTile::readTr4(io::SDLReader& reader)
{
  std::unique_ptr<TextureTile> tile{std::make_unique<TextureTile>()};
  tile->textureKey.blendingMode = static_cast<BlendingMode>(reader.readU16());
  tile->textureKey.tileAndFlag = reader.readU16();
  if((tile->textureKey.tileAndFlag & 0x7FFF) > 128)
    BOOST_LOG_TRIVIAL(warning) << "TR4 Object Texture: tileAndFlag > 128";

  tile->textureKey.flags = reader.readU16();
  tile->uvCoordinates[0] = UVCoordinates::readTr4(reader);
  tile->uvCoordinates[1] = UVCoordinates::readTr4(reader);
  tile->uvCoordinates[2] = UVCoordinates::readTr4(reader);
  tile->uvCoordinates[3] = UVCoordinates::readTr4(reader);
  tile->unknown1 = reader.readU32();
  tile->unknown2 = reader.readU32();
  tile->x_size = reader.readU32();
  tile->y_size = reader.readU32();
  return tile;
}

std::unique_ptr<TextureTile> TextureTile::readTr5(io::SDLReader& reader)
{
  std::unique_ptr<TextureTile> tile = readTr4(reader);
  if(reader.readU16() != 0)
  {
    BOOST_LOG_TRIVIAL(warning) << "TR5 Object Texture: unexpected value at end of structure";
  }
  return tile;
}
} // namespace loader::file
