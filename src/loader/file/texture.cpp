#include "texture.h"

#include "io/sdlreader.h"

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <gl/image.h>
#include <gl/pixel.h>

namespace loader::file
{
void DWordTexture::toImage() const
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
    sizeof(textile->pixels));
  return textile;
}

std::unique_ptr<WordTexture> WordTexture::read(io::SDLReader& reader)
{
  auto texture = std::make_unique<WordTexture>();

  for(auto& row : texture->pixels)
  {
    std::generate(row.begin(),
                  row.end(),
                  [&reader]()
                  {
                    return reader.readU16();
                  });
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
  tile->textureKey.atlasIdAndFlag = reader.readU16();
  if(tile->textureKey.atlasIdAndFlag > 64)
    BOOST_LOG_TRIVIAL(warning) << "TR1 Object Texture: tileAndFlag > 64";

  if((tile->textureKey.atlasIdAndFlag & (1u << 15u)) != 0)
    BOOST_LOG_TRIVIAL(warning) << "TR1 Object Texture: tileAndFlag is flagged";

  std::generate(tile->uvCoordinates.begin(),
                tile->uvCoordinates.end(),
                [&reader]()
                {
                  return UVCoordinates::read(reader);
                });
  return tile;
}

std::unique_ptr<TextureTile> TextureTile::readTr4(io::SDLReader& reader)
{
  auto tile = std::make_unique<TextureTile>();
  tile->textureKey.blendingMode = static_cast<BlendingMode>(reader.readU16());
  tile->textureKey.atlasIdAndFlag = reader.readU16();
  if((tile->textureKey.atlasIdAndFlag & 0x7FFFu) > 128)
    BOOST_LOG_TRIVIAL(warning) << "TR4 Object Texture: tileAndFlag > 128";

  tile->textureKey.flags = reader.readU16();
  std::generate(tile->uvCoordinates.begin(),
                tile->uvCoordinates.end(),
                [&reader]()
                {
                  return UVCoordinates::read(reader);
                });
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
