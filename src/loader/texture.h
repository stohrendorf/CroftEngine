#pragma once

#include "io/sdlreader.h"
#include "gameplay.h"

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>


namespace loader
{
namespace trx
{
class Glidos;
}


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
* Each pixel is a colour with the following format.<br>
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

        for( auto& row : texture->pixels )
        {
            for( int j = 0; j < 256; j++ )
            {
                row[j] = reader.readU16();
            }
        }

        return texture;
    }
};


struct DWordTexture final
{
    gameplay::gl::RGBA8 pixels[256][256];

    std::string md5;


    static std::unique_ptr<DWordTexture> read(io::SDLReader& reader)
    {
        std::unique_ptr<DWordTexture> textile{new DWordTexture()};

        for( auto& row : textile->pixels )
        {
            for( int j = 0; j < 256; j++ )
            {
                auto tmp = reader.readU32(); // format is ARGB
                const uint8_t a = (tmp >> 24) & 0xff;
                const uint8_t r = (tmp >> 16) & 0xff;
                const uint8_t g = (tmp >> 8) & 0xff;
                const uint8_t b = (tmp >> 0) & 0xff;
                row[j] = {r, g, b, a};
            }
        }

        return textile;
    }


    std::shared_ptr<gameplay::gl::Texture> toTexture(trx::Glidos* glidos, const boost::filesystem::path& lvlName) const;

    std::shared_ptr<gameplay::gl::Image<gameplay::gl::RGBA8>> toImage(trx::Glidos* glidos, const boost::filesystem::path& lvlName) const;
};


enum class BlendingMode
    : uint16_t
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


    /// \brief reads object texture vertex definition.
    static UVCoordinates readTr1(io::SDLReader& reader)
    {
        UVCoordinates vert;
        vert.xcoordinate = reader.readI8();
        vert.xpixel = reader.readU8();
        vert.ycoordinate = reader.readI8();
        vert.ypixel = reader.readU8();
        return vert;
    }


    static UVCoordinates readTr4(io::SDLReader& reader)
    {
        UVCoordinates vert;
        vert.xcoordinate = reader.readI8();
        vert.xpixel = reader.readU8();
        vert.ycoordinate = reader.readI8();
        vert.ypixel = reader.readU8();
        if( vert.xcoordinate == 0 )
        {
            vert.xcoordinate = 1;
        }
        if( vert.ycoordinate == 0 )
        {
            vert.ycoordinate = 1;
        }
        return vert;
    }


    glm::vec2 toGl() const
    {
        return glm::vec2{(xpixel+0.5f) / 256.0f, (ypixel+0.5f) / 256.0f};
    }
};


extern std::shared_ptr<gameplay::Material> createMaterial(const std::shared_ptr<gameplay::gl::Texture>& texture,
                                                          BlendingMode bmode,
                                                          const std::shared_ptr<gameplay::ShaderProgram>& shader);


struct TextureLayoutProxy
{
    struct TextureKey
    {
        BlendingMode blendingMode = BlendingMode::Solid;

        // 0 means that a texture is all-opaque, and that transparency
        // information is ignored.
        // 1 means that transparency information is used. In 8-bit colour,
        // index 0 is the transparent colour, while in 16-bit colour, the
        // top bit (0x8000) is the alpha channel (1 = opaque, 0 = transparent).
        // 2 (only in TR3) means that the opacity (alpha) is equal to the intensity;
        // the brighter the colour, the more opaque it is. The intensity is probably calculated
        // as the maximum of the individual color values.
        uint16_t tileAndFlag = 0; // index into textile list

        uint16_t flags = 0; // TR4

        int colorId = -1;


        bool operator==(const TextureKey& rhs) const
        {
            return tileAndFlag == rhs.tileAndFlag
                   && flags == rhs.flags
                   && blendingMode == rhs.blendingMode
                   && colorId == rhs.colorId;
        }


        bool operator<(const TextureKey& rhs) const
        {
            if( tileAndFlag != rhs.tileAndFlag )
            {
                return tileAndFlag < rhs.tileAndFlag;
            }

            if( flags != rhs.flags )
            {
                return flags < rhs.flags;
            }

            if( blendingMode != rhs.blendingMode )
            {
                return blendingMode < rhs.blendingMode;
            }

            return colorId < rhs.colorId;
        }
    };


    TextureKey textureKey;

    UVCoordinates uvCoordinates[4]; // the four corners of the texture
    uint32_t unknown1; // TR4
    uint32_t unknown2; // TR4
    uint32_t x_size; // TR4
    uint32_t y_size; // TR4

    /** \brief reads object texture definition.
    *
    * some sanity checks get done and if they fail an exception gets thrown.
    * all values introduced in TR4 get set appropiatly.
    */
    static std::unique_ptr<TextureLayoutProxy> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<TextureLayoutProxy> proxy{new TextureLayoutProxy()};
        proxy->textureKey.blendingMode = static_cast<BlendingMode>(reader.readU16());
        proxy->textureKey.tileAndFlag = reader.readU16();
        if( proxy->textureKey.tileAndFlag > 64 )
        BOOST_LOG_TRIVIAL(warning) << "TR1 Object Texture: tileAndFlag > 64";

        if( (proxy->textureKey.tileAndFlag & (1 << 15)) != 0 )
        BOOST_LOG_TRIVIAL(warning) << "TR1 Object Texture: tileAndFlag is flagged";

        // only in TR4
        proxy->textureKey.flags = 0;
        proxy->uvCoordinates[0] = UVCoordinates::readTr1(reader);
        proxy->uvCoordinates[1] = UVCoordinates::readTr1(reader);
        proxy->uvCoordinates[2] = UVCoordinates::readTr1(reader);
        proxy->uvCoordinates[3] = UVCoordinates::readTr1(reader);
        // only in TR4
        proxy->unknown1 = 0;
        proxy->unknown2 = 0;
        proxy->x_size = 0;
        proxy->y_size = 0;
        return proxy;
    }


    static std::unique_ptr<TextureLayoutProxy> readTr4(io::SDLReader& reader)
    {
        std::unique_ptr<TextureLayoutProxy> proxy{new TextureLayoutProxy()};
        proxy->textureKey.blendingMode = static_cast<BlendingMode>(reader.readU16());
        proxy->textureKey.tileAndFlag = reader.readU16();
        if( (proxy->textureKey.tileAndFlag & 0x7FFF) > 128 )
        BOOST_LOG_TRIVIAL(warning) << "TR4 Object Texture: tileAndFlag > 128";

        proxy->textureKey.flags = reader.readU16();
        proxy->uvCoordinates[0] = UVCoordinates::readTr4(reader);
        proxy->uvCoordinates[1] = UVCoordinates::readTr4(reader);
        proxy->uvCoordinates[2] = UVCoordinates::readTr4(reader);
        proxy->uvCoordinates[3] = UVCoordinates::readTr4(reader);
        proxy->unknown1 = reader.readU32();
        proxy->unknown2 = reader.readU32();
        proxy->x_size = reader.readU32();
        proxy->y_size = reader.readU32();
        return proxy;
    }


    static std::unique_ptr<TextureLayoutProxy> readTr5(io::SDLReader& reader)
    {
        std::unique_ptr<TextureLayoutProxy> proxy = readTr4(reader);
        if( reader.readU16() != 0 )
        {
            BOOST_LOG_TRIVIAL(warning) << "TR5 Object Texture: unexpected value at end of structure";
        }
        return proxy;
    }


    std::shared_ptr<gameplay::Material> createMaterial(const std::shared_ptr<gameplay::gl::Texture>& texture,
                                                       const std::shared_ptr<gameplay::ShaderProgram>& shader) const
    {
        return loader::createMaterial(texture, textureKey.blendingMode, shader);
    }
};
}
