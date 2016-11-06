#pragma once

#include "io/sdlreader.h"

#include <gsl/gsl>

#include "gameplay.h"

namespace loader
{
    /**
    * @brief 8-Bit RGBA color.
    * @ingroup native
    *
    * @note The alpha component is set from TR2 on only.
    */
    struct ByteColor
    {
        uint8_t r, g, b, a;

        static ByteColor readTr1(io::SDLReader& reader)
        {
            return read(reader, false);
        }

        static ByteColor readTr2(io::SDLReader& reader)
        {
            return read(reader, true);
        }

        glm::vec4 toGLColor() const
        {
            return glm::vec4{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
        }

    private:
        static ByteColor read(io::SDLReader& reader, bool withAlpha)
        {
            ByteColor colour;
            colour.r = reader.readU8() << 2;
            colour.g = reader.readU8() << 2;
            colour.b = reader.readU8() << 2;
            if( withAlpha )
                colour.a = reader.readU8() << 2;
            else
                colour.a = 255;
            return colour;
        }
    };

    /**
    * @brief 32-Bit float RGBA color.
    * @ingroup native
    */
    struct FloatColor
    {
        float r, g, b, a;

        glm::vec4 toSColor(float intensity) const
        {
            BOOST_ASSERT(intensity >= 0 && intensity <= 1);
            glm::vec4 col;
            col.x = r * intensity;
            col.y = g * intensity;
            col.z = b * intensity;
            col.w = a * intensity;
            return col;
        }
    };

    struct Palette
    {
        ByteColor color[256];

        /// \brief reads the 256 colour palette values.
        static std::unique_ptr<Palette> readTr1(io::SDLReader& reader)
        {
            std::unique_ptr<Palette> palette{new Palette()};
            for( auto& c : gsl::span<ByteColor>(palette->color) )
                c = ByteColor::readTr1(reader);
            return palette;
        }

        static std::unique_ptr<Palette> readTr2(io::SDLReader& reader)
        {
            std::unique_ptr<Palette> palette{new Palette()};
            for( auto& c : gsl::span<ByteColor>(palette->color) )
                c = ByteColor::readTr2(reader);
            return palette;
        }
    };
}
