#pragma once

#include "io/sdlreader.h"

#include <irrlicht.h>
#include <gsl.h>

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

        irr::video::SColor toSColor() const
        {
            return irr::video::SColor(a, r, g, b);
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

        irr::video::SColor toSColor(irr::f32 intensity) const
        {
            BOOST_ASSERT(intensity >= 0 && intensity <= 1);
            irr::video::SColor col;
            col.setRed(gsl::narrow<irr::u8>(std::lround(r * intensity * 255)));
            col.setGreen(gsl::narrow<irr::u8>(std::lround(g * intensity * 255)));
            col.setBlue(gsl::narrow<irr::u8>(std::lround(b * intensity * 255)));
            col.setAlpha(gsl::narrow<irr::u8>(std::lround(a * intensity * 255)));
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
            for( auto& c : gsl::as_span(palette->color) )
                c = ByteColor::readTr1(reader);
            return palette;
        }

        static std::unique_ptr<Palette> readTr2(io::SDLReader& reader)
        {
            std::unique_ptr<Palette> palette{new Palette()};
            for( auto& c : gsl::as_span(palette->color) )
                c = ByteColor::readTr2(reader);
            return palette;
        }
    };
}
