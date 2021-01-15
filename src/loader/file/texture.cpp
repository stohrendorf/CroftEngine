#include "texture.h"

#include "engine/objects/object.h"
#include "io/sdlreader.h"
#include "loader/trx/trx.h"

namespace loader::file
{
void DWordTexture::toImage()
{
  image = std::make_shared<gl::Image<gl::SRGBA8>>(glm::ivec2{256, 256}, &pixels[0][0]);
}

std::unique_ptr<DWordTexture> DWordTexture::read(io::SDLReader& reader)
{
  auto texture = std::make_unique<DWordTexture>();

  for(auto& row : texture->pixels)
  {
    for(auto& element : row)
    {
      const auto tmp = reader.readU32(); // format is ARGB
      const uint8_t a = (tmp >> 24u) & 0xffu;
      const uint8_t r = (tmp >> 16u) & 0xffu;
      const uint8_t g = (tmp >> 8u) & 0xffu;
      const uint8_t b = (tmp >> 0u) & 0xffu;
      element = {r, g, b, a};
    }
  }

  return texture;
}

std::unique_ptr<ByteTexture> ByteTexture::read(io::SDLReader& reader)
{
  auto textile = std::make_unique<ByteTexture>();
  reader.readBytes(
    reinterpret_cast<uint8_t*>(&textile->pixels[0][0]), //NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    256 * 256);
  return textile;
}

std::unique_ptr<WordTexture> WordTexture::read(io::SDLReader& reader)
{
  auto texture = std::make_unique<WordTexture>();

  for(auto& row : texture->pixels)
  {
    std::generate(row.begin(), row.end(), [&reader]() { return reader.readU16(); });
  }

  return texture;
}

UVCoordinates UVCoordinates::read(io::SDLReader& reader)
{
  UVCoordinates uv{};
  uv.x = Component{reader.readU16()};
  uv.y = Component{reader.readU16()};
  return uv;
}

std::unique_ptr<TextureTile> TextureTile::readTr1(io::SDLReader& reader)
{
  auto tile = std::make_unique<TextureTile>();
  tile->textureKey.blendingMode = static_cast<BlendingMode>(reader.readU16());
  tile->textureKey.tileAndFlag = reader.readU16();
  if(tile->textureKey.tileAndFlag > 64)
    BOOST_LOG_TRIVIAL(warning) << "TR1 Object Texture: tileAndFlag > 64";

  if((tile->textureKey.tileAndFlag & (1u << 15u)) != 0)
    BOOST_LOG_TRIVIAL(warning) << "TR1 Object Texture: tileAndFlag is flagged";

  // only in TR4
  tile->textureKey.flags = 0;
  for(auto& uv : tile->uvCoordinates)
    uv = UVCoordinates::read(reader);
  // only in TR4
  tile->unknown1 = 0;
  tile->unknown2 = 0;
  tile->x_size = 0;
  tile->y_size = 0;
  return tile;
}

std::unique_ptr<TextureTile> TextureTile::readTr4(io::SDLReader& reader)
{
  auto tile = std::make_unique<TextureTile>();
  tile->textureKey.blendingMode = static_cast<BlendingMode>(reader.readU16());
  tile->textureKey.tileAndFlag = reader.readU16();
  if((tile->textureKey.tileAndFlag & 0x7FFFu) > 128)
    BOOST_LOG_TRIVIAL(warning) << "TR4 Object Texture: tileAndFlag > 128";

  tile->textureKey.flags = reader.readU16();
  for(auto& uv : tile->uvCoordinates)
    uv = UVCoordinates::read(reader);
  tile->unknown1 = reader.readU32();
  tile->unknown2 = reader.readU32();
  tile->x_size = reader.readU32();
  tile->y_size = reader.readU32();
  return tile;
}

std::unique_ptr<TextureTile> TextureTile::readTr5(io::SDLReader& reader)
{
  auto tile = readTr4(reader);
  if(reader.readU16() != 0)
  {
    BOOST_LOG_TRIVIAL(warning) << "TR5 Object Texture: unexpected value at end of structure";
  }
  return tile;
}
} // namespace loader::file
