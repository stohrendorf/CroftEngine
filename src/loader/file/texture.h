#pragma once

#include "core/id.h"
#include "io/sdlreader.h"
#include "render/gl/image.h"
#include "render/scene/Material.h"

#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>

namespace loader
{
namespace trx
{
class Glidos;
}

namespace file
{
struct ByteTexture
{
    uint8_t pixels[256][256];

    static std::unique_ptr<ByteTexture> read(io::SDLReader& reader)
    {
        std::unique_ptr<ByteTexture> textile{new ByteTexture()};
        reader.readBytes(reinterpret_cast<uint8_t*>(textile->pixels), 256 * 256);
        return textile;
    }
};

/** \brief 16-bit texture.
*
* Each pixel is a color with the following format.<br>
* - 1-bit transparency (0 ::= transparent, 1 ::= opaque) (0x8000)
* - 5-bit red channel (0x7c00)
* - 5-bit green channel (0x03e0)
* - 5-bit blue channel (0x001f)
*/
struct WordTexture
{
    uint16_t pixels[256][256];

    static std::unique_ptr<WordTexture> read(io::SDLReader& reader)
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
};

struct DWordTexture final
{
    render::gl::SRGBA8 pixels[256][256];
    std::shared_ptr<render::gl::Image<render::gl::SRGBA8>> image;
    std::shared_ptr<render::gl::Texture2D<render::gl::SRGBA8>> texture;

    std::string md5;

    static std::unique_ptr<DWordTexture> read(io::SDLReader& reader)
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

    void toTexture(const trx::Glidos* glidos, const std::function<void(const std::string&)>& statusCallback);

    void toImage(const trx::Glidos* glidos, const std::function<void(const std::string&)>& statusCallback);
};

enum class BlendingMode : uint16_t
{
    Solid,
    AlphaTransparency,
    VertexColorTransparency,
    SimpleShade,
    TransparentIgnoreZ,
    InvertSrc,
    Wireframe,
    TransparentAlpha,
    InvertDst,
    Screen,
    Hide,
    AnimatedTexture
};

/** \brief Object Texture Vertex.
*
* It specifies a vertex location in textile coordinates.
* The Xpixel and Ypixel are the actual coordinates of the vertex's pixel.
* The Xcoordinate and Ycoordinate values depend on where the other vertices
* are in the object texture. And if the object texture is used to specify
* a triangle, then the fourth vertex's values will all be zero.
*/
struct UVCoordinates
{
    int8_t xcoordinate; // 1 if Xpixel is the low value, -1 if Xpixel is the high value in the object texture
    uint8_t xpixel;

    int8_t ycoordinate; // 1 if Ypixel is the low value, -1 if Ypixel is the high value in the object texture
    uint8_t ypixel;

    constexpr bool operator==(const UVCoordinates& rhs) const noexcept
    {
        return xcoordinate == rhs.xcoordinate && xpixel == rhs.xpixel && ycoordinate == rhs.ycoordinate
               && ypixel == rhs.ypixel;
    }

    constexpr bool operator<(const UVCoordinates& rhs) const noexcept
    {
        if(xcoordinate != rhs.xcoordinate)
            return xcoordinate < rhs.xcoordinate;
        if(xpixel != rhs.xpixel)
            return xpixel < rhs.xpixel;
        if(ycoordinate != rhs.ycoordinate)
            return ycoordinate < rhs.ycoordinate;
        return ypixel < rhs.ypixel;
    }

    /// \brief reads object texture vertex definition.
    static UVCoordinates readTr1(io::SDLReader& reader)
    {
        UVCoordinates uv;
        uv.xcoordinate = reader.readI8();
        uv.xpixel = reader.readU8();
        uv.ycoordinate = reader.readI8();
        uv.ypixel = reader.readU8();
        return uv;
    }

    static UVCoordinates readTr4(io::SDLReader& reader)
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

    glm::vec2 toGl() const
    {
        return glm::vec2{(xpixel + 0.5f) / 256.0f, (ypixel + 0.5f) / 256.0f};
    }

    UVCoordinates& operator=(const glm::vec2& v)
    {
        xpixel = gsl::narrow<uint8_t>(std::lround(v.x * 256));
        ypixel = gsl::narrow<uint8_t>(std::lround(v.y * 256));
        return *this;
    }
};

extern gsl::not_null<std::shared_ptr<render::scene::Material>>
    createMaterial(const gsl::not_null<std::shared_ptr<render::gl::Texture>>& texture,
                   BlendingMode bmode,
                   const gsl::not_null<std::shared_ptr<render::scene::ShaderProgram>>& shader);

struct TextureKey
{
    BlendingMode blendingMode = BlendingMode::Solid;

    // 0 means that a texture is all-opaque, and that transparency
    // information is ignored.
    // 1 means that transparency information is used. In 8-bit color,
    // index 0 is the transparent color, while in 16-bit color, the
    // top bit (0x8000) is the alpha channel (1 = opaque, 0 = transparent).
    // 2 (only in TR3) means that the opacity (alpha) is equal to the intensity;
    // the brighter the color, the more opaque it is. The intensity is probably calculated
    // as the maximum of the individual color values.
    uint16_t tileAndFlag = 0; // index into textile list

    uint16_t flags = 0; // TR4

    DECLARE_ID(ColorId, int);
    ColorId colorId{-1};

    bool operator==(const TextureKey& rhs) const
    {
        return tileAndFlag == rhs.tileAndFlag && flags == rhs.flags && blendingMode == rhs.blendingMode
               && colorId == rhs.colorId;
    }

    bool operator<(const TextureKey& rhs) const
    {
        if(tileAndFlag != rhs.tileAndFlag)
        {
            return tileAndFlag < rhs.tileAndFlag;
        }

        if(flags != rhs.flags)
        {
            return flags < rhs.flags;
        }

        if(blendingMode != rhs.blendingMode)
        {
            return blendingMode < rhs.blendingMode;
        }

        return colorId.get() < rhs.colorId.get();
    }
};

struct TextureTile
{
    TextureKey textureKey;

    std::array<UVCoordinates, 4> uvCoordinates; // the four corners of the texture
    uint32_t unknown1;                          // TR4
    uint32_t unknown2;                          // TR4
    uint32_t x_size;                            // TR4
    uint32_t y_size;                            // TR4

    bool operator==(const TextureTile& rhs) const
    {
        return textureKey == rhs.textureKey && uvCoordinates == rhs.uvCoordinates && unknown1 == rhs.unknown1
               && unknown2 == rhs.unknown2 && x_size == rhs.x_size && y_size == rhs.y_size;
    }

    bool operator<(const TextureTile& rhs) const
    {
        if(!(textureKey == rhs.textureKey))
            return textureKey < rhs.textureKey;

        if(unknown1 != rhs.unknown1)
            return unknown1 < rhs.unknown1;

        if(unknown2 != rhs.unknown2)
            return unknown2 < rhs.unknown2;

        if(x_size != rhs.x_size)
            return x_size < rhs.x_size;

        return y_size < rhs.y_size;
    }

    /** \brief reads object texture definition.
    *
    * some sanity checks get done and if they fail an exception gets thrown.
    * all values introduced in TR4 get set appropriately.
    */
    static std::unique_ptr<TextureTile> readTr1(io::SDLReader& reader)
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

    static std::unique_ptr<TextureTile> readTr4(io::SDLReader& reader)
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

    static std::unique_ptr<TextureTile> readTr5(io::SDLReader& reader)
    {
        std::unique_ptr<TextureTile> tile = readTr4(reader);
        if(reader.readU16() != 0)
        {
            BOOST_LOG_TRIVIAL(warning) << "TR5 Object Texture: unexpected value at end of structure";
        }
        return tile;
    }

    gsl::not_null<std::shared_ptr<render::scene::Material>>
        createMaterial(const gsl::not_null<std::shared_ptr<render::gl::Texture>>& texture,
                       const gsl::not_null<std::shared_ptr<render::scene::ShaderProgram>>& shader) const
    {
        return loader::file::createMaterial(texture, textureKey.blendingMode, shader);
    }
};
} // namespace file
} // namespace loader
