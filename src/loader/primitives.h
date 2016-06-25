#pragma once

#include "io/sdlreader.h"

namespace loader
{
    struct Triangle
    {
        //! Vertex buffer indices
        uint16_t vertices[3];
        uint16_t proxyId; /**< \brief object-texture index or colour index.
                             * If the triangle is textured, then this is an index into the object-texture list.
                             * If it's not textured, then the low 8 bit contain the index into the 256 colour palette
                             * and from TR2 on the high 8 bit contain the index into the 16 bit palette.
                             */
        uint16_t lighting; /**< \brief transparency flag & strength of the hilight (TR4-TR5).
                             * bit0 if set, then alpha channel = intensity (see attribute in tr2_object_texture).<br>
                             * bit1-7 is the strength of the hilight.
                             */

        static Triangle readTr1(io::SDLReader& reader)
        {
            return read(reader, false);
        }

        static Triangle readTr4(io::SDLReader& reader)
        {
            return read(reader, true);
        }

    private:
        static Triangle read(io::SDLReader& reader, bool withLighting)
        {
            Triangle meshface;
            meshface.vertices[0] = reader.readU16();
            meshface.vertices[1] = reader.readU16();
            meshface.vertices[2] = reader.readU16();
            meshface.proxyId = reader.readU16();
            if( withLighting )
                meshface.lighting = reader.readU16();
            else
                meshface.lighting = 0;
            return meshface;
        }
    };

    struct QuadFace
    {
        //! Vertex buffer indices
        uint16_t vertices[4];
        uint16_t proxyId; /**< \brief object-texture index or colour index.
                             * If the rectangle is textured, then this is an index into the object-texture list.
                             * If it's not textured, then the low 8 bit contain the index into the 256 colour palette
                             * and from TR2 on the high 8 bit contain the index into the 16 bit palette.
                             */
        uint16_t lighting; /**< \brief transparency flag & strength of the hilight (TR4-TR5).
                             *
                             * In TR4, objects can exhibit some kind of light reflection when seen from some particular angles.
                             * - bit0 if set, then alpha channel = intensity (see attribute in tr2_object_texture).
                             * - bit1-7 is the strength of the hilight.
                             */

        static QuadFace readTr1(io::SDLReader& reader)
        {
            return read(reader, false);
        }

        static QuadFace readTr4(io::SDLReader& reader)
        {
            return read(reader, true);
        }

    private:
        static QuadFace read(io::SDLReader& reader, bool withLighting)
        {
            QuadFace meshface;
            meshface.vertices[0] = reader.readU16();
            meshface.vertices[1] = reader.readU16();
            meshface.vertices[2] = reader.readU16();
            meshface.vertices[3] = reader.readU16();
            meshface.proxyId = reader.readU16();
            if( withLighting )
                meshface.lighting = reader.readU16();
            else
                meshface.lighting = 0;
            return meshface;
        }
    };
}
