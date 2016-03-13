#pragma once

#include "game.h"
#include "io/sdlreader.h"
#include "util/helpers.h"

#include <array>
#include <stdexcept>
#include <vector>
#include <map>
#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>

namespace loader
{
namespace
{
constexpr const uint16_t TextureIndexMaskTr4 = 0x7FFF;          // in some custom levels we need to use 0x7FFF flag
constexpr const uint16_t TextureIndexMask = 0x0FFF;
//constexpr const uint16_t TR_TEXTURE_SHAPE_MASK = 0x7000;          // still not used
constexpr const uint16_t TextureFlippedMask = 0x8000;
}

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

private:
    static ByteColor read(io::SDLReader& reader, bool withAlpha)
    {
        ByteColor colour;
        colour.r = reader.readU8() << 2;
        colour.g = reader.readU8() << 2;
        colour.b = reader.readU8() << 2;
        if(withAlpha)
            colour.a = reader.readU8() << 2;
        else
            colour.a = 255;
        return colour;
    }
};

struct FloatColor
{
    float r, g, b, a;
};

struct Vertex : public irr::core::vector3df
{
    static Vertex read16(io::SDLReader& reader)
    {
        Vertex vertex;
        // read vertex and change coordinate system
        vertex.X = static_cast<float>(reader.readI16());
        vertex.Y = static_cast<float>(-reader.readI16());
        vertex.Z = static_cast<float>(reader.readI16());
        return vertex;
    }

    static Vertex read32(io::SDLReader& reader)
    {
        Vertex vertex;
        // read vertex and change coordinate system
        vertex.X = static_cast<float>(reader.readI32());
        vertex.Y = static_cast<float>(-reader.readI32());
        vertex.Z = static_cast<float>(reader.readI32());
        return vertex;
    }

    static Vertex readF(io::SDLReader& reader)
    {
        Vertex vertex;
        vertex.X = reader.readF();
        vertex.Y = -reader.readF();
        vertex.Z = reader.readF();
        return vertex;
    }
    
    Vertex& operator+=(const irr::core::vector3df& rhs)
    {
        static_cast<irr::core::vector3df&>(*this) += rhs;
        return *this;
    }

    Vertex operator+(const irr::core::vector3df& rhs) const
    {
        auto tmp = *this;
        tmp += rhs;
        return tmp;
    }
};

struct Triangle
{
    uint16_t vertices[3];    ///< index into the appropriate list of vertices.
    uint16_t uvTexture;        /**< \brief object-texture index or colour index.
                               * If the triangle is textured, then this is an index into the object-texture list.
                               * If it's not textured, then the low 8 bit contain the index into the 256 colour palette
                               * and from TR2 on the high 8 bit contain the index into the 16 bit palette.
                               */
    uint16_t lighting;       /**< \brief transparency flag & strength of the hilight (TR4-TR5).
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
        meshface.uvTexture = reader.readU16();
        if(withLighting)
            meshface.lighting = reader.readU16();
        else
            meshface.lighting = 0;
        return meshface;
    }
};

struct QuadFace
{
    uint16_t uvCoordinates[4];    ///< index into the appropriate list of vertices.
    uint16_t uvTexture;        /**< \brief object-texture index or colour index.
                               * If the rectangle is textured, then this is an index into the object-texture list.
                               * If it's not textured, then the low 8 bit contain the index into the 256 colour palette
                               * and from TR2 on the high 8 bit contain the index into the 16 bit palette.
                               */
    uint16_t lighting;       /**< \brief transparency flag & strength of the hilight (TR4-TR5).
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
        meshface.uvCoordinates[0] = reader.readU16();
        meshface.uvCoordinates[1] = reader.readU16();
        meshface.uvCoordinates[2] = reader.readU16();
        meshface.uvCoordinates[3] = reader.readU16();
        meshface.uvTexture = reader.readU16();
        if(withLighting)
            meshface.lighting = reader.readU16();
        else
            meshface.lighting = 0;
        return meshface;
    }
};

struct ByteTexture
{
    uint8_t pixels[256][256];

    static std::unique_ptr<ByteTexture> read(io::SDLReader& reader)
    {
        std::unique_ptr<ByteTexture> textile{ new ByteTexture() };
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
        std::unique_ptr<WordTexture> texture{ new WordTexture() };

        for(int i = 0; i < 256; i++)
        {
            for(int j = 0; j < 256; j++)
                texture->pixels[i][j] = reader.readU16();
        }

        return texture;
    }
};

struct DWordTexture final
{
    irr::video::SColor pixels[256][256];

    static std::unique_ptr<DWordTexture> read(io::SDLReader& reader)
    {
        std::unique_ptr<DWordTexture> textile{ new DWordTexture() };

        for(int i = 0; i < 256; i++)
        {
            for(int j = 0; j < 256; j++)
            {
                auto tmp = reader.readU32(); // format is ARGB
                textile->pixels[i][j].set(tmp);
            }
        }

        return textile;
    }
    
    irr::video::ITexture* toTexture(irr::video::IVideoDriver* drv, int texIdx);
};

struct Portal
{
    uint16_t adjoining_room;     ///< \brief which room this portal leads to.
    Vertex normal;         /**< \brief which way the portal faces.
                                   * the normal points away from the adjacent room->
                                   * to be seen through, it must point toward the viewpoint.
                                   */
    Vertex vertices[4];    /**< \brief the corners of this portal.
                                   * the right-hand rule applies with respect to the normal.
                                   * if the right-hand-rule is not followed, the portal will
                                   * contain visual artifacts instead of a viewport to
                                   * Adjoiningroom->
                                   */

    static Portal read(io::SDLReader& reader, const irr::core::vector3df& offset)
    {
        Portal portal;
        portal.adjoining_room = reader.readU16();
        portal.normal = Vertex::read16(reader);
        portal.vertices[0] = Vertex::read16(reader) + offset;
        portal.vertices[1] = Vertex::read16(reader) + offset;
        portal.vertices[2] = Vertex::read16(reader) + offset;
        portal.vertices[3] = Vertex::read16(reader) + offset;
        if(util::fuzzyOne(portal.normal.X) && util::fuzzyZero(portal.normal.Y) && util::fuzzyZero(portal.normal.Z))
            return portal;
        if(util::fuzzyOne(-portal.normal.X) && util::fuzzyZero(portal.normal.Y) && util::fuzzyZero(portal.normal.Z))
            return portal;
        if(util::fuzzyZero(portal.normal.X) && util::fuzzyOne(portal.normal.Y) && util::fuzzyZero(portal.normal.Z))
            return portal;
        if(util::fuzzyZero(portal.normal.X) && util::fuzzyOne(-portal.normal.Y) && util::fuzzyZero(portal.normal.Z))
            return portal;
        if(util::fuzzyZero(portal.normal.X) && util::fuzzyZero(portal.normal.Y) && util::fuzzyOne(portal.normal.Z))
            return portal;
        if(util::fuzzyZero(portal.normal.X) && util::fuzzyZero(portal.normal.Y) && util::fuzzyOne(-portal.normal.Z))
            return portal;
        // std::cerr << "read_tr_room_portal: normal not on world axis");
        return portal;
    }
};

struct Sector
{
    uint16_t floorDataIndex;     // Index into FloorData[]
    uint16_t boxIndex;    // Index into Boxes[]/Zones[] (-1 if none)
    uint8_t roomBelow;    // The number of the room below this one (-1 or 255 if none)
    int8_t floorHeight;          // Absolute height of floor (multiply by 256 for world coordinates)
    uint8_t roomAbove;    // The number of the room above this one (-1 or 255 if none)
    int8_t ceilingHeight;        // Absolute height of ceiling (multiply by 256 for world coordinates)

    static Sector read(io::SDLReader& reader)
    {
        Sector sector;
        sector.floorDataIndex = reader.readU16();
        sector.boxIndex = reader.readU16();
        sector.roomBelow = reader.readU8();
        sector.floorHeight = reader.readI8();
        sector.roomAbove = reader.readU8();
        sector.ceilingHeight = reader.readI8();
        return sector;
    }
};

/*
* lights
*/
enum class LightType : uint8_t
{
    Null,
    Point,
    Spotlight,
    Sun,
    Shadow
};

struct Light
{
    Vertex position;           // world coords
    ByteColor color;         // three bytes rgb values
    float intensity;            // Calculated intensity
    uint16_t intensity1;        // Light intensity
    uint16_t intensity2;        // Almost always equal to Intensity1 [absent from TR1 data files]
    uint32_t fade1;             // Falloff value 1
    uint32_t fade2;             // Falloff value 2 [absent from TR1 data files]
    uint8_t light_type;         // same as D3D (i.e. 2 is for spotlight)
    uint8_t unknown;            // always 0xff?
    float r_inner;
    float r_outer;
    float length;
    float cutoff;
    Vertex dir;           // direction
    Vertex pos2;          // world coords
    Vertex dir2;          // direction

    LightType getLightType() const
    {
        switch(light_type)
        {
            case 0:
                return LightType::Sun;
            case 1:
                return LightType::Point;
            case 2:
                return LightType::Spotlight;
            case 3:
                return LightType::Shadow;
            default:
                return LightType::Null;
        }
    }

    /** \brief reads a room light definition.
      *
      * intensity1 gets converted, so it matches the 0-32768 range introduced in TR3.
      * intensity2 and fade2 are introduced in TR2 and are set to intensity1 and fade1 for TR1.
      */
    static Light readTr1(io::SDLReader& reader)
    {
        Light light;
        light.position = Vertex::read32(reader);
        // read and make consistent
        const auto tmp = reader.readI16();
        std::cerr << "tmp=" << tmp << "\n";
        const uint16_t tmp2 = std::abs(tmp);
        BOOST_ASSERT(tmp2 >= 0 && tmp2 < 8192);
        light.intensity1 = (8191 - tmp2) << 2;
        light.fade1 = reader.readU32();
        // only in TR2
        light.intensity2 = light.intensity1;

        light.intensity = light.intensity1;
        light.intensity /= 4096.0f;

        if(light.intensity > 1.0f)
            light.intensity = 1.0f;

        light.fade2 = light.fade1;

        light.r_outer = static_cast<float>(light.fade1);
        light.r_inner = static_cast<float>(light.fade1 / 2);

        light.light_type = 1; // Point light

                                 // all white
        light.color.r = 0xff;
        light.color.g = 0xff;
        light.color.b = 0xff;
        light.color.a = 0xff;
        return light;
    }

    static Light readTr2(io::SDLReader& reader)
    {
        Light light;
        light.position = Vertex::read32(reader);
        light.intensity1 = reader.readU16();
        light.intensity2 = reader.readU16();
        light.fade1 = reader.readU32();
        light.fade2 = reader.readU32();

        light.intensity = light.intensity1;
        light.intensity /= 4096.0f;

        if(light.intensity > 1.0f)
            light.intensity = 1.0f;

        light.r_outer = static_cast<float>(light.fade1);
        light.r_inner = static_cast<float>(light.fade1 / 2);

        light.light_type = 1; // Point light

                                 // all white
        light.color.r = 0xff;
        light.color.g = 0xff;
        light.color.b = 0xff;
        return light;
    }

    static Light readTr3(io::SDLReader& reader)
    {
        Light light;
        light.position = Vertex::read32(reader);
        light.color.r = reader.readU8();
        light.color.g = reader.readU8();
        light.color.b = reader.readU8();
        light.color.a = reader.readU8();
        light.fade1 = reader.readU32();
        light.fade2 = reader.readU32();

        light.intensity = 1.0f;

        light.r_outer = static_cast<float>(light.fade1);
        light.r_inner = static_cast<float>(light.fade1) / 2.0f;

        light.light_type = 1; // Point light
        return light;
    }

    static Light readTr4(io::SDLReader& reader)
    {
        Light light;
        light.position = Vertex::read32(reader);
        light.color = ByteColor::readTr1(reader);
        light.light_type = reader.readU8();
        light.unknown = reader.readU8();
        light.intensity1 = reader.readU8();
        light.intensity = light.intensity1;
        light.intensity /= 32;
        light.r_inner = reader.readF();
        light.r_outer = reader.readF();
        light.length = reader.readF();
        light.cutoff = reader.readF();
        light.dir = Vertex::readF(reader);
        return light;
    }

    static Light readTr5(io::SDLReader& reader)
    {
        Light light;
        light.position = Vertex::readF(reader);
        //read_tr_colour(src, light.color);
        light.color.r = static_cast<uint8_t>(reader.readF() * 255);    // r
        light.color.g = static_cast<uint8_t>(reader.readF() * 255);    // g
        light.color.b = static_cast<uint8_t>(reader.readF() * 255);    // b
        light.color.a = static_cast<uint8_t>(reader.readF() * 255);    // a
        light.intensity = 1.0f;
        /*
        if ((temp != 0) && (temp != 0xCDCDCDCD))
        BOOST_THROW_EXCEPTION( TR_ReadError("read_tr5_room_light: seperator1 has wrong value") );
        */
        light.r_inner = reader.readF();
        light.r_outer = reader.readF();
        reader.readF();    // rad_input
        reader.readF();    // rad_output
        reader.readF();    // range
        light.dir = Vertex::readF(reader);
        light.pos2 = Vertex::read32(reader);
        light.dir2 = Vertex::read32(reader);
        light.light_type = reader.readU8();

        auto temp = reader.readU8();
        if(temp != 0xCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room Light: seperator2 has wrong value";

        temp = reader.readU8();
        if(temp != 0xCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room Light: seperator3 has wrong value";

        temp = reader.readU8();
        if(temp != 0xCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room Light: seperator4 has wrong value";

        return light;
    }
};

struct Sprite
{
    uint16_t vertex;                 // offset into vertex list
    uint16_t texture;                // offset into sprite texture list

    /// \brief reads a room sprite definition.
    static Sprite read(io::SDLReader& reader)
    {
        Sprite room_sprite;
        room_sprite.vertex = reader.readU16();
        room_sprite.texture = reader.readU16();
        return room_sprite;
    }
};

/** \brief Room layer (TR5).
  */
struct Layer
{
    uint16_t num_vertices;
    uint16_t unknown_l1;
    uint16_t unknown_l2;
    uint16_t num_rectangles;
    uint16_t num_triangles;
    uint16_t unknown_l3;
    uint16_t unknown_l4;
    //  The following 6 floats define the bounding box for the layer
    float bounding_box_x1;
    float bounding_box_y1;
    float bounding_box_z1;
    float bounding_box_x2;
    float bounding_box_y2;
    float bounding_box_z2;
    int16_t unknown_l6a;
    int16_t unknown_l6b;
    int16_t unknown_l7a;
    int16_t unknown_l7b;
    int16_t unknown_l8a;
    int16_t unknown_l8b;

    static Layer read(io::SDLReader& reader)
    {
        Layer layer;
        layer.num_vertices = reader.readU16();
        layer.unknown_l1 = reader.readU16();
        layer.unknown_l2 = reader.readU16();
        layer.num_rectangles = reader.readU16();
        layer.num_triangles = reader.readU16();
        layer.unknown_l3 = reader.readU16();
        layer.unknown_l4 = reader.readU16();
        if(reader.readU16() != 0)
            BOOST_LOG_TRIVIAL(warning) << "Room Layer: filler2 has wrong value";

        layer.bounding_box_x1 = reader.readF();
        layer.bounding_box_y1 = -reader.readF();
        layer.bounding_box_z1 = -reader.readF();
        layer.bounding_box_x2 = reader.readF();
        layer.bounding_box_y2 = -reader.readF();
        layer.bounding_box_z2 = -reader.readF();
        if(reader.readU32() != 0)
            BOOST_LOG_TRIVIAL(warning) << "Room Layer: filler3 has wrong value";

        layer.unknown_l6a = reader.readI16();
        layer.unknown_l6b = reader.readI16();
        layer.unknown_l7a = reader.readI16();
        layer.unknown_l7b = reader.readI16();
        layer.unknown_l8a = reader.readI16();
        layer.unknown_l8b = reader.readI16();
        return layer;
    }
};

struct RoomVertex
{
    Vertex vertex;    // where this vertex lies (relative to tr2_room_info::x/z)
    int16_t lighting1;
    uint16_t attributes;    // A set of flags for special rendering effects [absent from TR1 data files]
    // 0x8000 something to do with water surface
    // 0x4000 under water lighting modulation and
    // movement if viewed from above water surface
    // 0x2000 water/quicksand surface movement
    // 0x0010 "normal"
    int16_t lighting2;      // Almost always equal to Lighting1 [absent from TR1 data files]
    // TR5 -->
    Vertex normal;
    irr::video::SColor color;

    /** \brief reads a room vertex definition.
      *
      * lighting1 gets converted, so it matches the 0-32768 range introduced in TR3.
      * lighting2 is introduced in TR2 and is set to lighting1 for TR1.
      * attributes is introduced in TR2 and is set 0 for TR1.
      * All other values are introduced in TR5 and get set to appropiate values.
      */
    static RoomVertex readTr1(io::SDLReader& reader)
    {
        RoomVertex room_vertex;
        room_vertex.vertex = Vertex::read16(reader);
        // read and make consistent
        int tmp = reader.readU16();
        BOOST_ASSERT(tmp < 8192);
        room_vertex.lighting1 = (32768 - tmp*4);
        // only in TR2
        room_vertex.lighting2 = room_vertex.lighting1;
        room_vertex.attributes = 0;
        // only in TR5
        room_vertex.normal.set(0,0,0);
        auto f = room_vertex.lighting1 / 32768.0f * 255;
        room_vertex.color.set(255, f, f, f);
        return room_vertex;
    }

    static RoomVertex readTr2(io::SDLReader& reader)
    {
        RoomVertex room_vertex;
        room_vertex.vertex = Vertex::read16(reader);
        // read and make consistent
        room_vertex.lighting1 = (8191 - reader.readI16()) << 2;
        room_vertex.attributes = reader.readU16();
        room_vertex.lighting2 = (8191 - reader.readI16()) << 2;
        // only in TR5
        room_vertex.normal.set(0,0,0);
        auto f = room_vertex.lighting2 / 32768.0f * 255;
        room_vertex.color.set(255, f, f, f);
        return room_vertex;
    }

    static RoomVertex readTr3(io::SDLReader& reader)
    {
        RoomVertex room_vertex;
        room_vertex.vertex = Vertex::read16(reader);
        // read and make consistent
        room_vertex.lighting1 = reader.readI16();
        room_vertex.attributes = reader.readU16();
        room_vertex.lighting2 = reader.readI16();
        // only in TR5
        room_vertex.normal.set(0,0,0);
        room_vertex.color.set(255,
                              ((room_vertex.lighting2 & 0x7C00) >> 10) / 62.0f * 255,
                              ((room_vertex.lighting2 & 0x03E0) >> 5) / 62.0f * 255,
                              (room_vertex.lighting2 & 0x001F) / 62.0f * 255);
        return room_vertex;
    }

    static RoomVertex readTr4(io::SDLReader& reader)
    {
        RoomVertex room_vertex;
        room_vertex.vertex = Vertex::read16(reader);
        // read and make consistent
        room_vertex.lighting1 = reader.readI16();
        room_vertex.attributes = reader.readU16();
        room_vertex.lighting2 = reader.readI16();
        // only in TR5
        room_vertex.normal.set(0,0,0);

        room_vertex.color.set(255,
                              ((room_vertex.lighting2 & 0x7C00) >> 10) / 31.0f * 255,
                              ((room_vertex.lighting2 & 0x03E0) >> 5) / 31.0f * 255,
                              (room_vertex.lighting2 & 0x001F) / 31.0f * 255);
        return room_vertex;
    }

    static RoomVertex readTr5(io::SDLReader& reader)
    {
        RoomVertex vert;
        vert.vertex = Vertex::readF(reader);
        vert.normal = Vertex::readF(reader);
        auto b = reader.readU8();
        auto g = reader.readU8();
        auto r = reader.readU8();
        auto a = reader.readU8();
        vert.color.set(a,r,g,b);
        return vert;
    }
};

struct RoomStaticMesh
{
    Vertex position;       // world coords
    irr::f32 rotation;         // high two bits (0xC000) indicate steps of
    // 90 degrees (e.g. (Rotation >> 14) * 90)
    int16_t intensity1;     // Constant lighting; -1 means use mesh lighting
    int16_t intensity2;     // Like Intensity 1, and almost always the same value [absent from TR1 data files]
    uint16_t object_id;     // which StaticMesh item to draw
    FloatColor tint;      // extracted from intensity

    /** \brief reads a room staticmesh definition.
      *
      * rotation gets converted to float and scaled appropiatly.
      * intensity1 gets converted, so it matches the 0-32768 range introduced in TR3.
      * intensity2 is introduced in TR2 and is set to intensity1 for TR1.
      */
    static RoomStaticMesh readTr1(io::SDLReader& reader)
    {
        RoomStaticMesh room_static_mesh;
        room_static_mesh.position = Vertex::read32(reader);
        room_static_mesh.rotation = static_cast<float>(reader.readU16()) / 16384.0f * -90;
        room_static_mesh.intensity1 = reader.readI16();
        room_static_mesh.object_id = reader.readU16();
        // make consistent
        if(room_static_mesh.intensity1 >= 0)
            room_static_mesh.intensity1 = (8191 - room_static_mesh.intensity1) << 2;
        // only in TR2
        room_static_mesh.intensity2 = room_static_mesh.intensity1;

        room_static_mesh.tint.b = room_static_mesh.tint.g = room_static_mesh.tint.r = room_static_mesh.intensity2 / 16384.0f;
        room_static_mesh.tint.a = 1.0f;
        return room_static_mesh;
    }

    static RoomStaticMesh readTr2(io::SDLReader& reader)
    {
        RoomStaticMesh room_static_mesh;
        room_static_mesh.position = Vertex::read32(reader);
        room_static_mesh.rotation = static_cast<float>(reader.readU16()) / 16384.0f * -90;
        room_static_mesh.intensity1 = reader.readI16();
        room_static_mesh.intensity2 = reader.readI16();
        room_static_mesh.object_id = reader.readU16();
        // make consistent
        if(room_static_mesh.intensity1 >= 0)
            room_static_mesh.intensity1 = (8191 - room_static_mesh.intensity1) << 2;
        if(room_static_mesh.intensity2 >= 0)
            room_static_mesh.intensity2 = (8191 - room_static_mesh.intensity2) << 2;

        room_static_mesh.tint.b = room_static_mesh.tint.g = room_static_mesh.tint.r = room_static_mesh.intensity2 / 16384.0f;
        room_static_mesh.tint.a = 1.0f;
        return room_static_mesh;
    }

    static RoomStaticMesh readTr3(io::SDLReader& reader)
    {
        RoomStaticMesh room_static_mesh;
        room_static_mesh.position = Vertex::read32(reader);
        room_static_mesh.rotation = static_cast<float>(reader.readU16()) / 16384.0f * -90;
        room_static_mesh.intensity1 = reader.readI16();
        room_static_mesh.intensity2 = reader.readI16();
        room_static_mesh.object_id = reader.readU16();

        room_static_mesh.tint.r = (room_static_mesh.intensity1 & 0x001F) / 62.0f;

        room_static_mesh.tint.g = ((room_static_mesh.intensity1 & 0x03E0) >> 5) / 62.0f;

        room_static_mesh.tint.b = ((room_static_mesh.intensity1 & 0x7C00) >> 10) / 62.0f;
        room_static_mesh.tint.a = 1.0f;
        return room_static_mesh;
    }

    static RoomStaticMesh readTr4(io::SDLReader& reader)
    {
        RoomStaticMesh room_static_mesh;
        room_static_mesh.position = Vertex::read32(reader);
        room_static_mesh.rotation = static_cast<float>(reader.readU16()) / 16384.0f * -90;
        room_static_mesh.intensity1 = reader.readI16();
        room_static_mesh.intensity2 = reader.readI16();
        room_static_mesh.object_id = reader.readU16();

        room_static_mesh.tint.r = (room_static_mesh.intensity1 & 0x001F) / 31.0f;

        room_static_mesh.tint.g = ((room_static_mesh.intensity1 & 0x03E0) >> 5) / 31.0f;

        room_static_mesh.tint.b = ((room_static_mesh.intensity1 & 0x7C00) >> 10) / 31.0f;
        room_static_mesh.tint.a = 1.0f;
        return room_static_mesh;
    }
};

// In TR3-5, there were 5 reverb / echo effect flags for each
// room, but they were never used in PC versions - however, level
// files still contain this info, so we now can re-use these flags
// to assign reverb/echo presets to each room->
// Also, underwater environment can be considered as additional
// reverb flag, so overall amount is 6.

enum class ReverbType : uint8_t
{
    Outside,         // EFX_REVERB_PRESET_CITY
    SmallRoom,       // EFX_REVERB_PRESET_LIVINGROOM
    MediumRoom,      // EFX_REVERB_PRESET_WOODEN_LONGPASSAGE
    LargeRoom,       // EFX_REVERB_PRESET_DOME_TOMB
    Pipe,            // EFX_REVERB_PRESET_PIPE_LARGE
    Water,           // EFX_REVERB_PRESET_UNDERWATER
    Sentinel
};

/** \brief Object Texture.
  *
  * These, thee contents of ObjectTextures[], are used for specifying texture
  * mapping for the world geometry and for mesh objects.
  */
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
struct UVVertex
{
    int8_t xcoordinate;     // 1 if Xpixel is the low value, -1 if Xpixel is the high value in the object texture
    uint8_t xpixel;
    int8_t ycoordinate;     // 1 if Ypixel is the low value, -1 if Ypixel is the high value in the object texture
    uint8_t ypixel;

    /// \brief reads object texture vertex definition.
    static UVVertex readTr1(io::SDLReader& reader)
    {
        UVVertex vert;
        vert.xcoordinate = reader.readI8();
        vert.xpixel = reader.readU8();
        vert.ycoordinate = reader.readI8();
        vert.ypixel = reader.readU8();
        return vert;
    }

    static UVVertex readTr4(io::SDLReader& reader)
    {
        UVVertex vert;
        vert.xcoordinate = reader.readI8();
        vert.xpixel = reader.readU8();
        vert.ycoordinate = reader.readI8();
        vert.ypixel = reader.readU8();
        if(vert.xcoordinate == 0)
            vert.xcoordinate = 1;
        if(vert.ycoordinate == 0)
            vert.ycoordinate = 1;
        return vert;
    }
};

struct UVTexture
{
    struct TextureKey
    {
        BlendingMode blendingMode;
        // 0 means that a texture is all-opaque, and that transparency
        // information is ignored.
        // 1 means that transparency information is used. In 8-bit colour,
        // index 0 is the transparent colour, while in 16-bit colour, the
        // top bit (0x8000) is the alpha channel (1 = opaque, 0 = transparent).
        // 2 (only in TR3) means that the opacity (alpha) is equal to the intensity;
        // the brighter the colour, the more opaque it is. The intensity is probably calculated
        // as the maximum of the individual color values.
        uint16_t tileAndFlag;                     // index into textile list

        uint16_t flags;                             // TR4
        
        int colorId = -1;
        
        inline bool operator==(const TextureKey& rhs) const
        {
            return tileAndFlag == rhs.tileAndFlag
                && flags == rhs.flags
                && blendingMode == rhs.blendingMode
                && colorId == rhs.colorId;
        }
        
        inline bool operator<(const TextureKey& rhs) const
        {
            if( tileAndFlag != rhs.tileAndFlag )
                return tileAndFlag < rhs.tileAndFlag;
            
            if( flags != rhs.flags )
                return flags < rhs.flags;
            
            if( blendingMode != rhs.blendingMode )
                return blendingMode < rhs.blendingMode;

            return colorId < rhs.colorId;
        }
    };
    
    TextureKey textureKey;
    UVVertex vertices[4];      // the four corners of the texture
    uint32_t unknown1;                          // TR4
    uint32_t unknown2;                          // TR4
    uint32_t x_size;                            // TR4
    uint32_t y_size;                            // TR4

    /** \brief reads object texture definition.
      *
      * some sanity checks get done and if they fail an exception gets thrown.
      * all values introduced in TR4 get set appropiatly.
      */
    static std::unique_ptr<UVTexture> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<UVTexture> object_texture{ new UVTexture() };
        object_texture->textureKey.blendingMode = static_cast<BlendingMode>(reader.readU16());
        object_texture->textureKey.tileAndFlag = reader.readU16();
        if(object_texture->textureKey.tileAndFlag > 64)
            BOOST_LOG_TRIVIAL(warning) << "TR1 Object Texture: tileAndFlag > 64";

        if((object_texture->textureKey.tileAndFlag & (1 << 15)) != 0)
            BOOST_LOG_TRIVIAL(warning) << "TR1 Object Texture: tileAndFlag is flagged";

        // only in TR4
        object_texture->textureKey.flags = 0;
        object_texture->vertices[0] = UVVertex::readTr1(reader);
        object_texture->vertices[1] = UVVertex::readTr1(reader);
        object_texture->vertices[2] = UVVertex::readTr1(reader);
        object_texture->vertices[3] = UVVertex::readTr1(reader);
        // only in TR4
        object_texture->unknown1 = 0;
        object_texture->unknown2 = 0;
        object_texture->x_size = 0;
        object_texture->y_size = 0;
        return object_texture;
    }

    static std::unique_ptr<UVTexture> readTr4(io::SDLReader& reader)
    {
        std::unique_ptr<UVTexture> object_texture{ new UVTexture() };
        object_texture->textureKey.blendingMode = static_cast<BlendingMode>(reader.readU16());
        object_texture->textureKey.tileAndFlag = reader.readU16();
        if((object_texture->textureKey.tileAndFlag & 0x7FFF) > 128)
            BOOST_LOG_TRIVIAL(warning) << "TR4 Object Texture: tileAndFlag > 128";

        object_texture->textureKey.flags = reader.readU16();
        object_texture->vertices[0] = UVVertex::readTr4(reader);
        object_texture->vertices[1] = UVVertex::readTr4(reader);
        object_texture->vertices[2] = UVVertex::readTr4(reader);
        object_texture->vertices[3] = UVVertex::readTr4(reader);
        object_texture->unknown1 = reader.readU32();
        object_texture->unknown2 = reader.readU32();
        object_texture->x_size = reader.readU32();
        object_texture->y_size = reader.readU32();
        return object_texture;
    }

    static std::unique_ptr<UVTexture> readTr5(io::SDLReader& reader)
    {
        std::unique_ptr<UVTexture> object_texture = readTr4(reader);
        if(reader.readU16() != 0)
        {
            BOOST_LOG_TRIVIAL(warning) << "TR5 Object Texture: unexpected value at end of structure";
        }
        return object_texture;
    }

    static irr::video::SMaterial createMaterial(irr::video::ITexture* texture, BlendingMode bmode)
    {
        irr::video::SMaterial result;
        // Set some defaults
        result.setTexture(0, texture);
        result.BackfaceCulling = false;
        result.ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
        result.Lighting = true;
        result.AmbientColor.set(0);
        result.TextureLayer[0].TextureWrapU = irr::video::ETC_CLAMP;
        result.TextureLayer[0].TextureWrapV = irr::video::ETC_CLAMP;

        switch(bmode)
        {
            case BlendingMode::Solid:
                result.BlendOperation = irr::video::EBO_ADD;
                break;

            case BlendingMode::AlphaTransparency:
                result.MaterialType = irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL;
                result.BlendOperation = irr::video::EBO_ADD;
                break;

            case BlendingMode::VertexColorTransparency:                                    // Classic PC alpha
                result.MaterialType = irr::video::EMT_TRANSPARENT_VERTEX_ALPHA;
                result.BlendOperation = irr::video::EBO_ADD;
                break;

            case BlendingMode::InvertSrc:                                  // Inversion by src (PS darkness) - SAME AS IN TR3-TR5
                result.BlendOperation = irr::video::EBO_SUBTRACT;
                break;

            case BlendingMode::InvertDst:                                 // Inversion by dest
                result.BlendOperation = irr::video::EBO_REVSUBTRACT;
                break;

            case BlendingMode::Screen:                                      // Screen (smoke, etc.)
                result.BlendOperation = irr::video::EBO_SUBTRACT;
                result.MaterialType = irr::video::EMT_TRANSPARENT_ADD_COLOR;
                break;

            case BlendingMode::AnimatedTexture:
                break;

            default:                                             // opaque animated textures case
                BOOST_ASSERT(false); // FIXME [irrlicht]
        }
        
        return result;
    }
};

struct Mesh
{
    Vertex center;                // This is usually close to the mesh's centroid, and appears to be the center of a sphere used for collision testing.
    int32_t collision_size;             // This appears to be the radius of that aforementioned collisional sphere.
    std::vector<Vertex> vertices;             //[NumVertices]; // list of vertices (relative coordinates)
    std::vector<Vertex> normals;              //[NumNormals]; // list of normals (if NumNormals is positive)
    std::vector<int16_t> lights;                    //[-NumNormals]; // list of light values (if NumNormals is negative)
    std::vector<QuadFace> textured_rectangles;   //[NumTexturedRectangles]; // list of textured rectangles
    std::vector<Triangle> textured_triangles;    //[NumTexturedTriangles]; // list of textured triangles
    // the rest is not present in TR4
    std::vector<QuadFace> colored_rectangles;   //[NumColouredRectangles]; // list of coloured rectangles
    std::vector<Triangle> colored_triangles;    //[NumColouredTriangles]; // list of coloured triangles

    /** \brief reads mesh definition.
      *
      * The read num_normals value is positive when normals are available and negative when light
      * values are available. The values get set appropiatly.
      */
    static std::unique_ptr<Mesh> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<Mesh> mesh{ new Mesh() };
        mesh->center = Vertex::read16(reader);
        mesh->collision_size = reader.readI32();

        mesh->vertices.resize(reader.readI16());
        for(size_t i = 0; i < mesh->vertices.size(); i++)
            mesh->vertices[i] = Vertex::read16(reader);

        auto num_normals = reader.readI16();
        if(num_normals >= 0)
        {
            mesh->normals.resize(num_normals);
            for(size_t i = 0; i < mesh->normals.size(); i++)
                mesh->normals[i] = Vertex::read16(reader);
        }
        else
        {
            mesh->lights.resize(-num_normals);
            for(size_t i = 0; i < mesh->lights.size(); i++)
                mesh->lights[i] = reader.readI16();
        }

        mesh->textured_rectangles.resize(reader.readI16());
        for(size_t i = 0; i < mesh->textured_rectangles.size(); i++)
            mesh->textured_rectangles[i] = QuadFace::readTr1(reader);

        mesh->textured_triangles.resize(reader.readI16());
        for(size_t i = 0; i < mesh->textured_triangles.size(); i++)
            mesh->textured_triangles[i] = Triangle::readTr1(reader);

        mesh->colored_rectangles.resize(reader.readI16());
        for(size_t i = 0; i < mesh->colored_rectangles.size(); i++)
            mesh->colored_rectangles[i] = QuadFace::readTr1(reader);

        mesh->colored_triangles.resize(reader.readI16());
        for(size_t i = 0; i < mesh->colored_triangles.size(); i++)
            mesh->colored_triangles[i] = Triangle::readTr1(reader);
        return mesh;
    }

    static std::unique_ptr<Mesh> readTr4(io::SDLReader& reader)
    {
        std::unique_ptr<Mesh> mesh{ new Mesh() };
        mesh->center = Vertex::read16(reader);
        mesh->collision_size = reader.readI32();

        mesh->vertices.resize(reader.readI16());
        for(size_t i = 0; i < mesh->vertices.size(); i++)
            mesh->vertices[i] = Vertex::read16(reader);

        auto num_normals = reader.readI16();
        if(num_normals >= 0)
        {
            mesh->normals.resize(num_normals);
            for(size_t i = 0; i < mesh->normals.size(); i++)
                mesh->normals[i] = Vertex::read16(reader);
        }
        else
        {
            mesh->lights.resize(-num_normals);
            for(size_t i = 0; i < mesh->lights.size(); i++)
                mesh->lights[i] = reader.readI16();
        }

        mesh->textured_rectangles.resize(reader.readI16());
        for(size_t i = 0; i < mesh->textured_rectangles.size(); i++)
            mesh->textured_rectangles[i] = QuadFace::readTr4(reader);

        mesh->textured_triangles.resize(reader.readI16());
        for(size_t i = 0; i < mesh->textured_triangles.size(); i++)
            mesh->textured_triangles[i] = Triangle::readTr4(reader);
        return mesh;
    }

    irr::scene::SMesh* createMesh(irr::scene::ISceneManager* mgr, int dumpIdx, const std::vector<UVTexture>& uvTextures, const std::map<UVTexture::TextureKey, irr::video::SMaterial>& materials, const std::vector<irr::video::SMaterial>& colorMaterials) const;
};

class Level;

struct Room
{
    irr::scene::ISceneNode* node = nullptr;

    // Various room flags specify various room options. Mostly, they
    // specify environment type and some additional actions which should
    // be performed in such rooms.
    static constexpr uint16_t TR_ROOM_FLAG_WATER          = 0x0001;
    static constexpr uint16_t TR_ROOM_FLAG_QUICKSAND      = 0x0002;  // Moved from 0x0080 to avoid confusion with NL.
    static constexpr uint16_t TR_ROOM_FLAG_SKYBOX         = 0x0008;
    static constexpr uint16_t TR_ROOM_FLAG_UNKNOWN1       = 0x0010;
    static constexpr uint16_t TR_ROOM_FLAG_WIND           = 0x0020;
    static constexpr uint16_t TR_ROOM_FLAG_UNKNOWN2       = 0x0040;  ///< @FIXME: Find what it means!!! Always set by Dxtre3d.
    static constexpr uint16_t TR_ROOM_FLAG_NO_LENSFLARE   = 0x0080;  // In TR4-5. Was quicksand in TR3.
    static constexpr uint16_t TR_ROOM_FLAG_MIST           = 0x0100;  ///< @FIXME: Unknown meaning in TR1!!!
    static constexpr uint16_t TR_ROOM_FLAG_CAUSTICS       = 0x0200;
    static constexpr uint16_t TR_ROOM_FLAG_UNKNOWN3       = 0x0400;
    static constexpr uint16_t TR_ROOM_FLAG_DAMAGE         = 0x0800;  ///< @FIXME: Is it really damage (D)?
    static constexpr uint16_t TR_ROOM_FLAG_POISON         = 0x1000;  ///< @FIXME: Is it really poison (P)?
    
    Vertex position;
    float lowestHeight;
    float greatestHeight;
    std::vector<Layer> layers;
    std::vector<RoomVertex> vertices;
    std::vector<QuadFace> rectangles;
    std::vector<Triangle> triangles;
    std::vector<Sprite> sprites;
    std::vector<Portal> portals;
    uint16_t sectorCountZ;          // "width" of sector list
    uint16_t sectorCountX;          // "height" of sector list
    std::vector<Sector> sectors;  // [NumXsectors * NumZsectors] list of sectors in this room
    int16_t intensity1;             // This and the next one only affect externally-lit objects
    int16_t intensity2;             // Almost always the same value as AmbientIntensity1 [absent from TR1 data files]
    int16_t lightMode;             // (present only in TR2: 0 is normal, 1 is flickering(?), 2 and 3 are uncertain)
    std::vector<Light> lights;       // [NumLights] list of point lights
    std::vector<RoomStaticMesh> staticMeshes;    // [NumStaticMeshes]list of static meshes
    int16_t alternateRoom;         // number of the room that this room can alternate
    int8_t  alternateGroup;        // number of group which is used to switch alternate rooms
    // with (e.g. empty/filled with water is implemented as an empty room that alternates with a full room)

    uint16_t flags;
    // Flag bits:
    // 0x0001 - room is filled with water,
    // 0x0020 - Lara's ponytail gets blown by the wind;
    // TR1 has only the water flag and the extra unknown flag 0x0100.
    // TR3 most likely has flags for "is raining", "is snowing", "water is cold", and "is
    // filled by quicksand", among others.

    uint8_t waterScheme;
    // Water scheme is used with various room options, for example, R and M room flags in TRLE.
    // Also, it specifies lighting scheme, when 0x4000 vertex attribute is set.

    ReverbType reverbInfo;

    // Reverb info is used in TR3-5 and contains index that specifies reverb type.
    // 0 - Outside, 1 - Small room, 2 - Medium room, 3 - Large room, 4 - Pipe.

    FloatColor lightColor;    // Present in TR5 only

    // TR5 only:

    float room_x;
    float room_z;
    float room_y_bottom;
    float room_y_top;

    uint32_t unknown_r1;
    uint32_t unknown_r2;
    uint32_t unknown_r3;
    uint16_t unknown_r4a;
    uint16_t unknown_r4b;
    uint32_t unknown_r5;
    uint32_t unknown_r6;

    /** \brief reads a room definition.
      *
      * intensity1 gets converted, so it matches the 0-32768 range introduced in TR3.
      * intensity2 is introduced in TR2 and is set to intensity1 for TR1.
      * light_mode is only in TR2 and is set 0 for TR1.
      * light_colour is only in TR3-4 and gets set appropiatly.
      */
    static std::unique_ptr<Room> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<Room> room{ new Room() };

        // read and change coordinate system
        room->position.X = static_cast<float>(reader.readI32());
        room->position.Y = 0;
        room->position.Z = static_cast<float>(reader.readI32());
        room->lowestHeight = static_cast<float>(-reader.readI32());
        room->greatestHeight = static_cast<float>(-reader.readI32());

        auto num_data_words = reader.readU32();

        auto position = reader.tell();

        room->vertices.resize(reader.readU16());
        for(size_t i = 0; i < room->vertices.size(); i++)
            room->vertices[i] = RoomVertex::readTr1(reader);

        room->rectangles.resize(reader.readU16());
        for(size_t i = 0; i < room->rectangles.size(); i++)
            room->rectangles[i] = QuadFace::readTr1(reader);

        room->triangles.resize(reader.readU16());
        for(size_t i = 0; i < room->triangles.size(); i++)
            room->triangles[i] = Triangle::readTr1(reader);

        room->sprites.resize(reader.readU16());
        for(size_t i = 0; i < room->sprites.size(); i++)
            room->sprites[i] = Sprite::read(reader);

        // set to the right position in case that there is some unused data
        reader.seek(position + num_data_words * 2);

        room->portals.resize(reader.readU16());
        for(size_t i = 0; i < room->portals.size(); i++)
            room->portals[i] = Portal::read(reader, room->position);

        room->sectorCountZ = reader.readU16();
        room->sectorCountX = reader.readU16();
        room->sectors.resize(room->sectorCountZ * room->sectorCountX);
        for(uint32_t i = 0; i < static_cast<uint32_t>(room->sectorCountZ * room->sectorCountX); i++)
            room->sectors[i] = Sector::read(reader);

        // read and make consistent
        room->intensity1 = (8191 - reader.readI16()) << 2;
        // only in TR2-TR4
        room->intensity2 = room->intensity1;
        // only in TR2
        room->lightMode = 0;

        room->lights.resize(reader.readU16());
        for(size_t i = 0; i < room->lights.size(); i++)
            room->lights[i] = Light::readTr1(reader);

        room->staticMeshes.resize(reader.readU16());
        for(size_t i = 0; i < room->staticMeshes.size(); i++)
            room->staticMeshes[i] = RoomStaticMesh::readTr1(reader);

        room->alternateRoom = reader.readI16();
        room->alternateGroup = 0;   // Doesn't exist in TR1-3

        room->flags = reader.readU16();
        room->reverbInfo = ReverbType::MediumRoom;

        room->lightColor.r = room->intensity1 / 32767.0f;
        room->lightColor.g = room->intensity1 / 32767.0f;
        room->lightColor.b = room->intensity1 / 32767.0f;
        room->lightColor.a = 1.0f;
        return room;
    }

    static std::unique_ptr<Room> readTr2(io::SDLReader& reader)
    {
        std::unique_ptr<Room> room{ new Room() };
        // read and change coordinate system
        room->position.X = static_cast<float>(reader.readI32());
        room->position.Y = 0;
        room->position.Z = static_cast<float>(reader.readI32());
        room->lowestHeight = static_cast<float>(-reader.readI32());
        room->greatestHeight = static_cast<float>(-reader.readI32());

        auto num_data_words = reader.readU32();

        auto position = reader.tell();

        room->vertices.resize(reader.readU16());
        for(size_t i = 0; i < room->vertices.size(); i++)
            room->vertices[i] = RoomVertex::readTr2(reader);

        room->rectangles.resize(reader.readU16());
        for(size_t i = 0; i < room->rectangles.size(); i++)
            room->rectangles[i] = QuadFace::readTr1(reader);

        room->triangles.resize(reader.readU16());
        for(size_t i = 0; i < room->triangles.size(); i++)
            room->triangles[i] = Triangle::readTr1(reader);

        room->sprites.resize(reader.readU16());
        for(size_t i = 0; i < room->sprites.size(); i++)
            room->sprites[i] = Sprite::read(reader);

        // set to the right position in case that there is some unused data
        reader.seek(position + num_data_words * 2);

        room->portals.resize(reader.readU16());
        for(size_t i = 0; i < room->portals.size(); i++)
            room->portals[i] = Portal::read(reader, room->position);

        room->sectorCountZ = reader.readU16();
        room->sectorCountX = reader.readU16();
        room->sectors.resize(room->sectorCountZ * room->sectorCountX);
        for(size_t i = 0; i < static_cast<uint32_t>(room->sectorCountZ * room->sectorCountX); i++)
            room->sectors[i] = Sector::read(reader);

        // read and make consistent
        room->intensity1 = (8191 - reader.readI16()) << 2;
        room->intensity2 = (8191 - reader.readI16()) << 2;
        room->lightMode = reader.readI16();

        room->lights.resize(reader.readU16());
        for(size_t i = 0; i < room->lights.size(); i++)
            room->lights[i] = Light::readTr2(reader);

        room->staticMeshes.resize(reader.readU16());
        for(size_t i = 0; i < room->staticMeshes.size(); i++)
            room->staticMeshes[i] = RoomStaticMesh::readTr2(reader);

        room->alternateRoom = reader.readI16();
        room->alternateGroup = 0;   // Doesn't exist in TR1-3

        room->flags = reader.readU16();

        if(room->flags & 0x0020)
        {
            room->reverbInfo = ReverbType::Outside;
        }
        else
        {
            room->reverbInfo = ReverbType::MediumRoom;
        }

        room->lightColor.r = room->intensity1 / 16384.0f;
        room->lightColor.g = room->intensity1 / 16384.0f;
        room->lightColor.b = room->intensity1 / 16384.0f;
        room->lightColor.a = 1.0f;
        return room;
    }

    static std::unique_ptr<Room> readTr3(io::SDLReader& reader)
    {
        std::unique_ptr<Room> room{ new Room() };

        // read and change coordinate system
        room->position.X = static_cast<float>(reader.readI32());
        room->position.Y = 0;
        room->position.Z = static_cast<float>(reader.readI32());
        room->lowestHeight = static_cast<float>(-reader.readI32());
        room->greatestHeight = static_cast<float>(-reader.readI32());

        auto num_data_words = reader.readU32();

        auto position = reader.tell();

        room->vertices.resize(reader.readU16());
        for(size_t i = 0; i < room->vertices.size(); i++)
            room->vertices[i] = RoomVertex::readTr3(reader);

        room->rectangles.resize(reader.readU16());
        for(size_t i = 0; i < room->rectangles.size(); i++)
            room->rectangles[i] = QuadFace::readTr1(reader);

        room->triangles.resize(reader.readU16());
        for(size_t i = 0; i < room->triangles.size(); i++)
            room->triangles[i] = Triangle::readTr1(reader);

        room->sprites.resize(reader.readU16());
        for(size_t i = 0; i < room->sprites.size(); i++)
            room->sprites[i] = Sprite::read(reader);

        // set to the right position in case that there is some unused data
        reader.seek(position + num_data_words * 2);

        room->portals.resize(reader.readU16());
        for(size_t i = 0; i < room->portals.size(); i++)
            room->portals[i] = Portal::read(reader, room->position);

        room->sectorCountZ = reader.readU16();
        room->sectorCountX = reader.readU16();
        room->sectors.resize(room->sectorCountZ * room->sectorCountX);
        for(size_t i = 0; i < static_cast<uint32_t>(room->sectorCountZ * room->sectorCountX); i++)
            room->sectors[i] = Sector::read(reader);

        room->intensity1 = reader.readI16();
        room->intensity2 = reader.readI16();

        // only in TR2
        room->lightMode = 0;

        room->lights.resize(reader.readU16());
        for(size_t i = 0; i < room->lights.size(); i++)
            room->lights[i] = Light::readTr3(reader);

        room->staticMeshes.resize(reader.readU16());
        for(size_t i = 0; i < room->staticMeshes.size(); i++)
            room->staticMeshes[i] = RoomStaticMesh::readTr3(reader);

        room->alternateRoom = reader.readI16();
        room->alternateGroup = 0;   // Doesn't exist in TR1-3

        room->flags = reader.readU16();

        if(room->flags & 0x0080)
        {
            room->flags |= 0x0002;   // Move quicksand flag to another bit to avoid confusion with NL flag.
            room->flags ^= 0x0080;
        }

        // Only in TR3-5

        room->waterScheme = reader.readU8();
        room->reverbInfo = static_cast<ReverbType>(reader.readU8());

        reader.skip(1);   // Alternate_group override?

        room->lightColor.r = room->intensity1 / 65534.0f;
        room->lightColor.g = room->intensity1 / 65534.0f;
        room->lightColor.b = room->intensity1 / 65534.0f;
        room->lightColor.a = 1.0f;
        return room;
    }

    static std::unique_ptr<Room> readTr4(io::SDLReader& reader)
    {
        std::unique_ptr<Room> room{ new Room() };
        // read and change coordinate system
        room->position.X = static_cast<float>(reader.readI32());
        room->position.Y = 0;
        room->position.Z = static_cast<float>(reader.readI32());
        room->lowestHeight = static_cast<float>(-reader.readI32());
        room->greatestHeight = static_cast<float>(-reader.readI32());

        auto num_data_words = reader.readU32();

        auto position = reader.tell();

        room->vertices.resize(reader.readU16());
        for(size_t i = 0; i < room->vertices.size(); i++)
            room->vertices[i] = RoomVertex::readTr4(reader);

        room->rectangles.resize(reader.readU16());
        for(size_t i = 0; i < room->rectangles.size(); i++)
            room->rectangles[i] = QuadFace::readTr1(reader);

        room->triangles.resize(reader.readU16());
        for(size_t i = 0; i < room->triangles.size(); i++)
            room->triangles[i] = Triangle::readTr1(reader);

        room->sprites.resize(reader.readU16());
        for(size_t i = 0; i < room->sprites.size(); i++)
            room->sprites[i] = Sprite::read(reader);

        // set to the right position in case that there is some unused data
        reader.seek(position + num_data_words * 2);

        room->portals.resize(reader.readU16());
        for(size_t i = 0; i < room->portals.size(); i++)
            room->portals[i] = Portal::read(reader, room->position);

        room->sectorCountZ = reader.readU16();
        room->sectorCountX = reader.readU16();
        room->sectors.resize(room->sectorCountZ * room->sectorCountX);
        for(size_t i = 0; i < static_cast<uint32_t>(room->sectorCountZ * room->sectorCountX); i++)
            room->sectors[i] = Sector::read(reader);

        room->intensity1 = reader.readI16();
        room->intensity2 = reader.readI16();

        // only in TR2
        room->lightMode = 0;

        room->lights.resize(reader.readU16());
        for(size_t i = 0; i < room->lights.size(); i++)
            room->lights[i] = Light::readTr4(reader);

        room->staticMeshes.resize(reader.readU16());
        for(size_t i = 0; i < room->staticMeshes.size(); i++)
            room->staticMeshes[i] = RoomStaticMesh::readTr4(reader);

        room->alternateRoom = reader.readI16();
        room->flags = reader.readU16();

        // Only in TR3-5

        room->waterScheme = reader.readU8();
        room->reverbInfo = static_cast<ReverbType>(reader.readU8());

        // Only in TR4-5

        room->alternateGroup = reader.readU8();

        room->lightColor.r = (room->intensity2 & 0x00FF) / 255.0f;
        room->lightColor.g = ((room->intensity1 & 0xFF00) >> 8) / 255.0f;
        room->lightColor.b = (room->intensity1 & 0x00FF) / 255.0f;
        room->lightColor.a = ((room->intensity2 & 0xFF00) >> 8) / 255.0f;
        return room;
    }

    static std::unique_ptr<Room> readTr5(io::SDLReader& reader)
    {
        if(reader.readU32() != 0x414C4558)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: 'XELA' not found";

        const auto room_data_size = reader.readU32();
        const auto position = reader.tell();
        const auto endPos = position + room_data_size;

        std::unique_ptr<Room> room{ new Room() };
        room->intensity1 = 32767;
        room->intensity2 = 32767;
        room->lightMode = 0;

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator1 has wrong value";

        /*portal_offset = */reader.readI32();             // StartPortalOffset?   // endSDOffset
        auto sector_data_offset = reader.readU32();    // StartSDOffset
        auto temp = reader.readU32();
        if(temp != 0 && temp != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator2 has wrong value";

        auto static_meshes_offset = reader.readU32();     // endPortalOffset
                                                        // static_meshes_offset or room_layer_offset
                                                        // read and change coordinate system
        room->position.X = static_cast<float>(reader.readI32());
        room->position.Y = static_cast<float>(reader.readU32());
        room->position.Z = static_cast<float>(reader.readI32());
        room->lowestHeight = static_cast<float>(-reader.readI32());
        room->greatestHeight = static_cast<float>(-reader.readI32());

        room->sectorCountZ = reader.readU16();
        room->sectorCountX = reader.readU16();

        room->lightColor.b = reader.readU8() / 255.0f;
        room->lightColor.g = reader.readU8() / 255.0f;
        room->lightColor.r = reader.readU8() / 255.0f;
        room->lightColor.a = reader.readU8() / 255.0f;
        //room->light_colour.a = 1.0f;

        room->lights.resize(reader.readU16());
        if(room->lights.size() > 512)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: lights.size() > 512";

        room->staticMeshes.resize(reader.readU16());
        if(room->staticMeshes.size() > 512)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: static_meshes.size() > 512";

        room->reverbInfo = static_cast<ReverbType>(reader.readU8());
        room->alternateGroup = reader.readU8();
        room->waterScheme = static_cast<uint8_t>(reader.readU16());

        if(reader.readU32() != 0x00007FFF)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: filler1 has wrong value";

        if(reader.readU32() != 0x00007FFF)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: filler2 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator4 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator5 has wrong value";

        if(reader.readU32() != 0xFFFFFFFF)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator6 has wrong value";

        room->alternateRoom = reader.readI16();

        room->flags = reader.readU16();

        room->unknown_r1 = reader.readU32();
        room->unknown_r2 = reader.readU32();
        room->unknown_r3 = reader.readU32();

        temp = reader.readU32();
        if(temp != 0 && temp != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator7 has wrong value";

        room->unknown_r4a = reader.readU16();
        room->unknown_r4b = reader.readU16();

        room->room_x = reader.readF();
        room->unknown_r5 = reader.readU32();
        room->room_z = -reader.readF();

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator8 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator9 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator10 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator11 has wrong value";

        temp = reader.readU32();
        if(temp != 0 && temp != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator12 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator13 has wrong value";

        auto num_triangles = reader.readU32();
        if(num_triangles == 0xCDCDCDCD)
            num_triangles = 0;
        if(num_triangles > 512)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: triangles.size() > 512";
        room->triangles.resize(num_triangles);

        auto num_rectangles = reader.readU32();
        if(num_rectangles == 0xCDCDCDCD)
            num_rectangles = 0;
        if(num_rectangles > 1024)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: rectangles.size() > 1024";
        room->rectangles.resize(num_rectangles);

        if(reader.readU32() != 0)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator14 has wrong value";

        /*light_size = */reader.readU32();
        auto numL2 = reader.readU32();
        if(numL2 != room->lights.size())
            BOOST_THROW_EXCEPTION(std::runtime_error("TR5 Room: numLights2 != lights.size()"));

        room->unknown_r6 = reader.readU32();
        room->room_y_top = -reader.readF();
        room->room_y_bottom = -reader.readF();

        room->layers.resize(reader.readU32());

        auto layer_offset = reader.readU32();
        auto vertices_offset = reader.readU32();
        auto poly_offset = reader.readU32();
        auto poly_offset2 = reader.readU32();
        if(poly_offset != poly_offset2)
            BOOST_THROW_EXCEPTION(std::runtime_error("TR5 Room: poly_offset != poly_offset2"));

        auto vertices_size = reader.readU32();
        if(vertices_size % 28 != 0)
            BOOST_THROW_EXCEPTION(std::runtime_error("TR5 Room: vertices_size has wrong value"));

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator15 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator16 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator17 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator18 has wrong value";

        for(size_t i = 0; i < room->lights.size(); i++)
            room->lights[i] = Light::readTr5(reader);

        reader.seek(position + 208 + sector_data_offset);

        room->sectors.resize(room->sectorCountZ * room->sectorCountX);
        for(size_t i = 0; i < static_cast<uint32_t>(room->sectorCountZ * room->sectorCountX); i++)
            room->sectors[i] = Sector::read(reader);

        room->portals.resize(reader.readI16());
        for(size_t i = 0; i < room->portals.size(); i++)
            room->portals[i] = Portal::read(reader, room->position);

        reader.seek(position + 208 + static_meshes_offset);

        for(size_t i = 0; i < room->staticMeshes.size(); i++)
            room->staticMeshes[i] = RoomStaticMesh::readTr4(reader);

        reader.seek(position + 208 + layer_offset);

        for(size_t i = 0; i < room->layers.size(); i++)
            room->layers[i] = Layer::read(reader);

        reader.seek(position + 208 + poly_offset);

        {
            uint32_t vertex_index = 0;
            uint32_t rectangle_index = 0;
            uint32_t triangle_index = 0;

            for(size_t i = 0; i < room->layers.size(); i++)
            {
                uint32_t j;

                for(j = 0; j < room->layers[i].num_rectangles; j++)
                {
                    room->rectangles[rectangle_index] = QuadFace::readTr4(reader);
                    room->rectangles[rectangle_index].uvCoordinates[0] += vertex_index;
                    room->rectangles[rectangle_index].uvCoordinates[1] += vertex_index;
                    room->rectangles[rectangle_index].uvCoordinates[2] += vertex_index;
                    room->rectangles[rectangle_index].uvCoordinates[3] += vertex_index;
                    rectangle_index++;
                }
                for(j = 0; j < room->layers[i].num_triangles; j++)
                {
                    room->triangles[triangle_index] = Triangle::readTr4(reader);
                    room->triangles[triangle_index].vertices[0] += vertex_index;
                    room->triangles[triangle_index].vertices[1] += vertex_index;
                    room->triangles[triangle_index].vertices[2] += vertex_index;
                    triangle_index++;
                }
                vertex_index += room->layers[i].num_vertices;
            }
        }

        reader.seek(position + 208 + vertices_offset);

        {
            uint32_t vertex_index = 0;
            room->vertices.resize(vertices_size / 28);
            //int temp1 = room_data_size - (208 + vertices_offset + vertices_size);
            for(size_t i = 0; i < room->layers.size(); i++)
            {
                uint32_t j;

                for(j = 0; j < room->layers[i].num_vertices; j++)
                    room->vertices[vertex_index++] = RoomVertex::readTr5(reader);
            }
        }

        reader.seek(endPos);

        return room;
    }
    
    irr::scene::IMeshSceneNode* createSceneNode(irr::scene::ISceneManager* mgr, int dumpIdx, const Level& level, const std::map<UVTexture::TextureKey, irr::video::SMaterial>& materials, const std::vector<irr::video::ITexture*>& textures, const std::vector<irr::scene::SMesh*>& staticMeshes);
};

struct StaticMesh
{
    uint32_t object_id;             // Object Identifier (matched in Items[])
    uint16_t mesh;                  // mesh (offset into MeshPointers[])
    Vertex visibility_box[2];
    Vertex collision_box[2];
    uint16_t flags;                 // Meaning uncertain; it is usually 2, and is 3 for objects Lara can travel through,
    // like TR2's skeletons and underwater vegetation

    static std::unique_ptr<StaticMesh> read(io::SDLReader& reader)
    {
        std::unique_ptr<StaticMesh> mesh{ new StaticMesh() };
        mesh->object_id = reader.readU32();
        mesh->mesh = reader.readU16();

        mesh->visibility_box[0].X = static_cast<float>(reader.readI16());
        mesh->visibility_box[1].X = static_cast<float>(reader.readI16());
        mesh->visibility_box[0].Y = static_cast<float>(-reader.readI16());
        mesh->visibility_box[1].Y = static_cast<float>(-reader.readI16());
        mesh->visibility_box[0].Z = static_cast<float>(reader.readI16());
        mesh->visibility_box[1].Z = static_cast<float>(reader.readI16());

        mesh->collision_box[0].X = static_cast<float>(reader.readI16());
        mesh->collision_box[1].X = static_cast<float>(reader.readI16());
        mesh->collision_box[0].Y = static_cast<float>(-reader.readI16());
        mesh->collision_box[1].Y = static_cast<float>(-reader.readI16());
        mesh->collision_box[0].Z = static_cast<float>(reader.readI16());
        mesh->collision_box[1].Z = static_cast<float>(reader.readI16());

        mesh->flags = reader.readU16();
        return mesh;
    }
};

/** \brief MeshTree.
  *
  * MeshTree[] is actually groups of four bit32s. The first one is a
  * "flags" word;
  *    bit 1 (0x0002) indicates "put the parent mesh on the mesh stack";
  *    bit 0 (0x0001) indicates "take the top mesh off of the mesh stack and use as the parent mesh"
  * when set, otherwise "use the previous mesh are the parent mesh".
  * When both are present, the bit-0 operation is always done before the bit-1 operation; in effect, read the stack but do not change it.
  * The next three bit32s are X, Y, Z offsets of the mesh's origin from the parent mesh's origin.
  */
struct MeshTree
{
    uint32_t flags;
    Vertex offset;
};

/** \brief Frame.
  *
  * Frames indicates how composite meshes are positioned and rotated.
  * They work in conjunction with Animations[] and Bone2[].
  *
  * A given frame has the following format:
  *    short BB1x, BB1y, BB1z           // bounding box (low)
  *    short BB2x, BB2y, BB2z           // bounding box (high)
  *    short OffsetX, OffsetY, OffsetZ  // starting offset for this moveable
  *    (TR1 ONLY: short NumValues       // number of angle sets to follow)
  *    (TR2/3: NumValues is implicitly NumMeshes (from moveable))
  *
  * What follows next is a list of angle sets.  In TR2/3, an angle set can
  * specify either one or three axes of rotation.  If either of the high two
  * bits (0xc000) of the first angle unsigned short are set, it's one axis:
  *  only one  unsigned short,
  *  low 10 bits (0x03ff),
  *  scale is 0x100 == 90 degrees;
  * the high two  bits are interpreted as follows:
  *  0x4000 == X only, 0x8000 == Y only,
  *  0xC000 == Z only.
  *
  * If neither of the high bits are set, it's a three-axis rotation.  The next
  * 10 bits (0x3ff0) are the X rotation, the next 10 (including the following
  * unsigned short) (0x000f, 0xfc00) are the Y rotation,
  * the next 10 (0x03ff) are the Z rotation, same scale as
  * before (0x100 == 90 degrees).
  *
  * Rotations are performed in Y, X, Z order.
  * TR1 ONLY: All angle sets are two words and interpreted like the two-word
  * sets in TR2/3, EXCEPT that the word order is reversed.
  *
  */
  /*typedef struct {
      tr5_vertex_t bbox_low;
      tr5_vertex_t bbox_high;
      tr5_vertex_t offset;
      tr5_vertex_array_t rotations;
      int32_t byte_offset;
  } tr_frame_t;
  typedef prtl::array < tr_frame_t > tr_frame_array_t;*/

struct AnimatedModel
{
    uint32_t object_id;         // Item Identifier (matched in Items[])
    uint16_t meshCount;        // number of meshes in this object
    uint16_t firstMesh;     // starting mesh (offset into MeshPointers[])
    uint32_t boneTreeIndex;   // offset into MeshTree[]
    uint32_t meshPositionOffset;      // byte offset into Frames[] (divide by 2 for Frames[i])
    uint16_t animationIndex;   // offset into Animations[]

    struct FrameRange
    {
        const irr::u32 offset;
        const irr::u32 firstFrame;
        const irr::u32 lastFrame;
        
        FrameRange(irr::u32 o, irr::u32 f, irr::u32 l)
            : offset(o)
            , firstFrame(f+o)
            , lastFrame(l+o)
        {
        }
    };
    
    std::map<uint16_t, FrameRange> frameMapping;
    
    /** \brief reads a moveable definition.
      *
      * some sanity checks get done which throw a exception on failure.
      * frame_offset needs to be corrected later in TR_Level::read_tr_level.
      */
    static std::unique_ptr<AnimatedModel> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<AnimatedModel> moveable{ new AnimatedModel() };
        moveable->object_id = reader.readU32();
        moveable->meshCount = reader.readU16();
        moveable->firstMesh = reader.readU16();
        moveable->boneTreeIndex = reader.readU32();
        moveable->meshPositionOffset = reader.readU32();
        moveable->animationIndex = reader.readU16();
        return moveable;
    }

    static std::unique_ptr<AnimatedModel> readTr5(io::SDLReader& reader)
    {
        std::unique_ptr<AnimatedModel> moveable = readTr1(reader);
        if(reader.readU16() != 0xFFEF)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Moveable: filler has wrong value";
        return moveable;
    }
};

struct Item
{
    int16_t object_id;     // Object Identifier (matched in Moveables[], or SpriteSequences[], as appropriate)
    uint16_t room;          // which room contains this item
    Vertex position;       // world coords
    irr::f32 rotation;        // ((0xc000 >> 14) * 90) degrees
    int16_t intensity1;    // (constant lighting; -1 means use mesh lighting)
    int16_t intensity2;    // Like Intensity1, and almost always with the same value. [absent from TR1 data files]
    int16_t ocb;           // Object code bit - used for altering entity behaviour. Only in TR4-5.
    uint16_t flags;
    // 0x0100 indicates "initially invisible", 0x3e00 is Activation Mask
    // 0x3e00 indicates "open" or "activated";  these can be XORed with
    // related FloorData::FDlist fields (e.g. for switches)

    uint16_t getActivationMask() const
    {
        return (flags & 0x3e00) >> 9;
    }

    bool isInitiallyInvisible() const
    {
        return (flags & 0x0100) != 0;
    }

    /// \brief reads an item definition.
    static std::unique_ptr<Item> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<Item> item{ new Item() };
        item->object_id = reader.readI16();
        item->room = reader.readU16();
        item->position = Vertex::read32(reader);
        item->rotation = static_cast<float>(reader.readU16()) / 16384.0f * 90;
        item->intensity1 = reader.readU16();
        if(item->intensity1 >= 0)
            item->intensity1 = (8191 - item->intensity1) << 2;
        item->intensity2 = item->intensity1;
        item->ocb = 0;   // Not present in TR1!
        item->flags = reader.readU16();
        return item;
    }

    static std::unique_ptr<Item> readTr2(io::SDLReader& reader)
    {
        std::unique_ptr<Item> item{ new Item() };
        item->object_id = reader.readI16();
        item->room = reader.readU16();
        item->position = Vertex::read32(reader);
        item->rotation = static_cast<float>(reader.readU16()) / 16384.0f * 90;
        item->intensity1 = reader.readU16();
        if(item->intensity1 >= 0)
            item->intensity1 = (8191 - item->intensity1) << 2;
        item->intensity2 = reader.readU16();
        if(item->intensity2 >= 0)
            item->intensity2 = (8191 - item->intensity2) << 2;
        item->ocb = 0;   // Not present in TR2!
        item->flags = reader.readU16();
        return item;
    }

    static std::unique_ptr<Item> readTr3(io::SDLReader& reader)
    {
        std::unique_ptr<Item> item{ new Item() };
        item->object_id = reader.readI16();
        item->room = reader.readU16();
        item->position = Vertex::read32(reader);
        item->rotation = static_cast<float>(reader.readU16()) / 16384.0f * 90;
        item->intensity1 = reader.readU16();
        item->intensity2 = reader.readU16();
        item->ocb = 0;   // Not present in TR3!
        item->flags = reader.readU16();
        return item;
    }

    static std::unique_ptr<Item> readTr4(io::SDLReader& reader)
    {
        std::unique_ptr<Item> item{ new Item() };
        item->object_id = reader.readI16();
        item->room = reader.readU16();
        item->position = Vertex::read32(reader);
        item->rotation = static_cast<float>(reader.readU16()) / 16384.0f * 90;
        item->intensity1 = reader.readU16();
        item->intensity2 = item->intensity1;
        item->ocb = reader.readU16();
        item->flags = reader.readU16();
        return item;
    }
};

struct SpriteTexture
{
    uint16_t texture;
    irr::core::vector2df t0;
    irr::core::vector2df t1;

    int16_t         left_side;
    int16_t         top_side;
    int16_t         right_side;
    int16_t         bottom_side;

    /** \brief reads sprite texture definition.
      *
      * some sanity checks get done and if they fail an exception gets thrown.
      */
    static std::unique_ptr<SpriteTexture> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<SpriteTexture> sprite_texture{ new SpriteTexture() };

        sprite_texture->texture = reader.readU16();
        if(sprite_texture->texture > 64)
            BOOST_LOG_TRIVIAL(warning) << "TR1 Sprite Texture: tile > 64";

        int tx = reader.readU8();
        int ty = reader.readU8();
        int tw = reader.readU16();
        int th = reader.readU16();
        int tleft = reader.readI16();
        int ttop = reader.readI16();
        int tright = reader.readI16();
        int tbottom = reader.readI16();

        float w = tw / 256.0f;
        float h = th / 256.0f;
        sprite_texture->t0.X = tx/255.0f;
        sprite_texture->t0.Y = ty/255.0f;
        sprite_texture->t1.X = sprite_texture->t0.X + w/255.0f;
        sprite_texture->t1.Y = sprite_texture->t0.Y + h/255.0f;

        sprite_texture->left_side = tleft;
        sprite_texture->right_side = tright;
        sprite_texture->top_side = -tbottom;
        sprite_texture->bottom_side = -ttop;
        return sprite_texture;
    }

    static std::unique_ptr<SpriteTexture> readTr4(io::SDLReader& reader)
    {
        std::unique_ptr<SpriteTexture> sprite_texture{ new SpriteTexture() };
        sprite_texture->texture = reader.readU16();
        if(sprite_texture->texture > 128)
            BOOST_LOG_TRIVIAL(warning) << "TR4 Sprite Texture: tile > 128";

        int tx = reader.readU8();
        int ty = reader.readU8();
        int tw = reader.readU16();
        int th = reader.readU16();
        int tleft = reader.readI16();
        int ttop = reader.readI16();
        int tright = reader.readI16();
        int tbottom = reader.readI16();

        sprite_texture->t0.X = tleft/255.0f;
        sprite_texture->t0.Y = tright/255.0f;
        sprite_texture->t1.X = tbottom/255.0f;
        sprite_texture->t1.Y = ttop/255.0f;

        sprite_texture->left_side = tx;
        sprite_texture->right_side = tx + tw / 256;
        sprite_texture->bottom_side = ty;
        sprite_texture->top_side = ty + th / 256;
        return sprite_texture;
    }
    
    irr::core::matrix4 buildTextureMatrix() const
    {
        auto tscale = t1 - t0;
        BOOST_ASSERT(tscale.X > 0);
        BOOST_ASSERT(tscale.Y > 0);

        irr::core::matrix4 mat;
        mat.setTextureScale(tscale.X, tscale.Y);
        mat.setTextureTranslate(t0.X, t0.Y);
        
        return mat;
    }
};

struct SpriteSequence
{
    uint32_t object_id;     // Item identifier (matched in Items[])
    int16_t length;        // negative of "how many sprites are in this sequence"
    int16_t offset;        // where (in sprite texture list) this sequence starts

    /** \brief reads sprite sequence definition.
      *
      * length is negative when read and thus gets negated.
      */
    static std::unique_ptr<SpriteSequence> read(io::SDLReader& reader)
    {
        std::unique_ptr<SpriteSequence> sprite_sequence{ new SpriteSequence() };
        sprite_sequence->object_id = reader.readU32();
        sprite_sequence->length = -reader.readI16();
        sprite_sequence->offset = reader.readI16();
        return sprite_sequence;
    }
};

/** \brief animation->
  *
  * This describes each individual animation; these may be looped by specifying
  * the next animation to be itself. In TR2 and TR3, one must be careful when
  * parsing frames using the FrameSize value as the size of each frame, since
  * an animation's frame range may extend into the next animation's frame range,
  * and that may have a different FrameSize value.
  */
struct Animation
{
    uint32_t poseDataOffset;      // byte offset into Frames[] (divide by 2 for Frames[i])
    uint8_t stretchFactor;      // Slowdown factor of this animation
    uint8_t poseDataSize;         // number of bit16's in Frames[] used by this animation
    uint16_t state_id;

    int32_t speed;
    int32_t accelleration;

    int32_t lateralSpeed;      // new in TR4 -->
    int32_t lateralAccelleration;      // lateral speed and acceleration.

    uint16_t firstFrame;           // first frame in this animation
    uint16_t lastFrame;             // last frame in this animation (numframes = (End - Start) + 1)
    uint16_t nextAnimation;
    uint16_t nextFrame;

    uint16_t transitionsCount;
    uint16_t transitionsIndex;   // offset into StateChanges[]
    uint16_t animCommandCount;     // How many of them to use.
    uint16_t animCommandIndex;          // offset into AnimCommand[]
    
    constexpr size_t getKeyframeCount() const
    {
        return (lastFrame - firstFrame + stretchFactor) / stretchFactor;
    }

    constexpr size_t getFrameCount() const
    {
        return lastFrame - firstFrame + 1;
    }

    /// \brief reads an animation definition.
    static std::unique_ptr<Animation> readTr1(io::SDLReader& reader)
    {
        return read(reader, false);
    }

    static std::unique_ptr<Animation> readTr4(io::SDLReader& reader)
    {
        return read(reader, true);
    }

private:
    static std::unique_ptr<Animation> read(io::SDLReader& reader, bool withLateral)
    {
        std::unique_ptr<Animation> animation{ new Animation() };
        animation->poseDataOffset = reader.readU32();
        animation->stretchFactor = reader.readU8();
        if(animation->stretchFactor == 0)
            animation->stretchFactor = 1;
        animation->poseDataSize = reader.readU8();
        animation->state_id = reader.readU16();

        animation->speed = reader.readI32();
        animation->accelleration = reader.readI32();
        if(withLateral)
        {
            animation->lateralSpeed = reader.readI32();
            animation->lateralAccelleration = reader.readI32();
        }
        else
        {
            animation->lateralSpeed = 0;
            animation->lateralAccelleration = 0;
        }

        animation->firstFrame = reader.readU16();
        animation->lastFrame = reader.readU16();
        animation->nextAnimation = reader.readU16();
        animation->nextFrame = reader.readU16();

        animation->transitionsCount = reader.readU16();
        animation->transitionsIndex = reader.readU16();
        animation->animCommandCount = reader.readU16();
        animation->animCommandIndex = reader.readU16();
        return animation;
    }
};

/** \brief State Change.
  *
  * Each one contains the state to change to and which animation dispatches
  * to use; there may be more than one, with each separate one covering a different
  * range of frames.
  */
struct Transitions
{
    uint16_t stateId;
    uint16_t transitionCaseCount;       // number of ranges (seems to always be 1..5)
    uint16_t firstTransitionCase;       // Offset into AnimDispatches[]

    /// \brief reads an animation state change.
    static std::unique_ptr<Transitions> read(io::SDLReader& reader)
    {
        std::unique_ptr<Transitions> state_change{ new Transitions() };
        state_change->stateId = reader.readU16();
        state_change->transitionCaseCount = reader.readU16();
        state_change->firstTransitionCase = reader.readU16();
        return state_change;
    }
};

/** \brief Animation Dispatch.
  *
  * This specifies the next animation and frame to use; these are associated
  * with some range of frames. This makes possible such specificity as one
  * animation for left foot forward and another animation for right foot forward.
  */
struct TransitionCase
{
    int16_t firstFrame;          // Lowest frame that uses this range
    int16_t lastFrame;           // Highest frame (+1?) that uses this range
    int16_t targetAnimation;     // Animation to dispatch to
    int16_t targetFrame;         // Frame offset to dispatch to

    /// \brief reads an animation dispatch.
    static std::unique_ptr<TransitionCase> read(io::SDLReader& reader)
    {
        std::unique_ptr<TransitionCase> anim_dispatch{ new TransitionCase() };
        anim_dispatch->firstFrame = reader.readI16();
        anim_dispatch->lastFrame = reader.readI16();
        anim_dispatch->targetAnimation = reader.readI16();
        anim_dispatch->targetFrame = reader.readI16();
        return anim_dispatch;
    }
};

struct Box
{
    uint32_t zmin;          // sectors (* 1024 units)
    uint32_t zmax;
    uint32_t xmin;
    uint32_t xmax;
    int16_t true_floor;     // Y value (no scaling)
    int16_t overlap_index;  // index into Overlaps[]. The high bit is sometimes set; this
    // occurs in front of swinging doors and the like.

    static std::unique_ptr<Box> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<Box> box{ new Box() };
        box->zmax = -reader.readI32();
        box->zmin = -reader.readI32();
        box->xmin = reader.readI32();
        box->xmax = reader.readI32();
        box->true_floor = -reader.readI16();
        box->overlap_index = reader.readI16();
        return box;
    }

    static std::unique_ptr<Box> readTr2(io::SDLReader& reader)
    {
        std::unique_ptr<Box> box{ new Box() };
        box->zmax = -1024 * reader.readU8();
        box->zmin = -1024 * reader.readU8();
        box->xmin = 1024 * reader.readU8();
        box->xmax = 1024 * reader.readU8();
        box->true_floor = -reader.readI16();
        box->overlap_index = reader.readI16();
        return box;
    }
};

struct Zone
{
    uint16_t flyZoneNormal;
    std::vector<uint16_t> groundZonesNormal;
    uint16_t flyZoneAlternate;
    std::vector<uint16_t> groundZonesAlternate;

    static std::unique_ptr<Zone> readTr1(io::SDLReader& reader)
    {
        return read(reader, 2);
    }

    static std::unique_ptr<Zone> readTr2(io::SDLReader& reader)
    {
        return read(reader, 4);
    }

private:
    static std::unique_ptr<Zone> read(io::SDLReader& reader, int n)
    {
        std::unique_ptr<Zone> zone{ new Zone() };
        zone->flyZoneNormal = reader.readU16();
        for(int i = 0; i < n; ++i)
            zone->groundZonesNormal.emplace_back(reader.readU16());
        zone->flyZoneAlternate = reader.readU16();
        for(int i = 0; i < n; ++i)
            zone->groundZonesAlternate.emplace_back(reader.readU16());
        return zone;
    }
};

/** \brief SoundSource.
  *
  * This structure contains the details of continuous-sound sources. Although
  * a SoundSource object has a position, it has no room membership; the sound
  * seems to propagate omnidirectionally for about 10 horizontal-grid sizes
  * without regard for the presence of walls.
  */
struct SoundSource
{
    int32_t x;              // absolute X position of sound source (world coordinates)
    int32_t y;              // absolute Y position of sound source (world coordinates)
    int32_t z;              // absolute Z position of sound source (world coordinates)
    uint16_t sound_id;      // internal sound index
    uint16_t flags;         // 0x40, 0x80, or 0xc0

    static std::unique_ptr<SoundSource> read(io::SDLReader& reader)
    {
        std::unique_ptr<SoundSource> sound_source{ new SoundSource() };
        sound_source->x = reader.readI32();
        sound_source->y = reader.readI32();
        sound_source->z = reader.readI32();

        sound_source->sound_id = reader.readU16();
        sound_source->flags = reader.readU16();
        return sound_source;
    }
};

// Looped field is located at offset 6 of SoundDetail structure and
// combined with SampleIndexes value. This field is responsible for
// looping behaviour of each sound.
// L flag sets sound to continous looped state, while W flag waits
// for any sound with similar ID to finish, and only then plays it
// again. R flag rewinds sound, if sound with similar ID is being
// sent to sources.
enum class LoopType
{
    None,
    Forward,
    PingPong,
    Wait
};

/** \brief SoundDetails.
 *
 * SoundDetails (also called SampleInfos in native TR sources) are properties
 * for each sound index from SoundMap. It contains all crucial information
 * that is needed to play certain sample, except offset to raw wave buffer,
 * which is unnecessary, as it is managed internally by DirectSound.
 */
struct SoundDetails
{
    size_t sample;                       // Index into SampleIndices -- NOT USED IN TR4-5!!!
    uint16_t volume;                     // Global sample value
    uint16_t sound_range;                // Sound range
    uint16_t chance;                     // Chance to play
    int16_t pitch;                       // Pitch shift
    uint8_t num_samples_and_flags_1;     // Bits 0-1: Looped flag, bits 2-5: num samples, bits 6-7: UNUSED
    uint8_t flags_2;                     // Bit 4: UNKNOWN, bit 5: Randomize pitch, bit 6: randomize volume
                                         // All other bits in flags_2 are unused.

    LoopType getLoopType(Engine engine) const
    {
        if(engine == Engine::TR1)
        {
            switch(num_samples_and_flags_1 & 3)
            {
                case 1:
                    return LoopType::PingPong;
                case 2:
                    return LoopType::Forward;
                default:
                    return LoopType::None;
            }
        }
        else if(engine == Engine::TR2)
        {
            switch(num_samples_and_flags_1 & 3)
            {
                case 1:
                    return LoopType::PingPong;
                case 3:
                    return LoopType::Forward;
                default:
                    return LoopType::None;
            }
        }
        else
        {
            switch(num_samples_and_flags_1 & 3)
            {
                case 1:
                    return LoopType::Wait;
                case 2:
                    return LoopType::PingPong;
                case 3:
                    return LoopType::Forward;
                default:
                    return LoopType::None;
            }
        }
    }

    uint8_t getSampleCount() const
    {
        return (num_samples_and_flags_1 >> 2) & 0x0f;
    }

    bool useRandomPitch() const
    {
        return (flags_2 & 0x20) != 0;
    }

    bool useRandomVolume() const
    {
        return (flags_2 & 0x40) != 0;
    }

    // Default range and pitch values are required for compatibility with
    // TR1 and TR2 levels, as there is no such parameters in SoundDetails
    // structures.

    static constexpr const int DefaultRange = 8;
    static constexpr const float DefaultPitch = 1.0f;       // 0.0 - only noise

    static std::unique_ptr<SoundDetails> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<SoundDetails> sound_details{ new SoundDetails() };
        sound_details->sample = reader.readU16();
        sound_details->volume = reader.readU16();
        sound_details->chance = reader.readU16();
        sound_details->num_samples_and_flags_1 = reader.readU8();
        sound_details->flags_2 = reader.readU8();
        sound_details->sound_range = DefaultRange;
        sound_details->pitch = static_cast<int16_t>(DefaultPitch);
        return sound_details;
    }

    static std::unique_ptr<SoundDetails> readTr3(io::SDLReader& reader)
    {
        std::unique_ptr<SoundDetails> sound_details{ new SoundDetails() };
        sound_details->sample = reader.readU16();
        sound_details->volume = reader.readU8();
        sound_details->sound_range = reader.readU8();
        sound_details->chance = static_cast<uint16_t>(reader.readU8());
        sound_details->pitch = reader.readI8();
        sound_details->num_samples_and_flags_1 = reader.readU8();
        sound_details->flags_2 = reader.readU8();
        return sound_details;
    }
};

struct AnimatedTexture
{
    std::vector<int16_t> texture_ids; // offsets into ObjectTextures[], in animation order.
};       //[NumAnimatedTextures];

struct Camera
{
    int32_t x;
    int32_t y;
    int32_t z;
    int16_t room;
    uint16_t unknown1;    // correlates to Boxes[]? Zones[]?

    static std::unique_ptr<Camera> read(io::SDLReader& reader)
    {
        std::unique_ptr<Camera> camera{ new Camera() };
        camera->x = reader.readI32();
        camera->y = reader.readI32();
        camera->z = reader.readI32();

        camera->room = reader.readI16();
        camera->unknown1 = reader.readU16();
        return camera;
    }
};

struct FlybyCamera
{
    int32_t  cam_x;
    int32_t  cam_y;
    int32_t  cam_z;
    int32_t  target_x;
    int32_t  target_y;
    int32_t  target_z;
    uint8_t  sequence;
    uint8_t  index;
    uint16_t fov;
    uint16_t roll;
    uint16_t timer;
    uint16_t speed;
    uint16_t flags;
    uint32_t room_id;

    static std::unique_ptr<FlybyCamera> read(io::SDLReader& reader)
    {
        std::unique_ptr<FlybyCamera> camera{ new FlybyCamera() };
        camera->cam_x = reader.readI32();
        camera->cam_y = reader.readI32();
        camera->cam_z = reader.readI32();
        camera->target_x = reader.readI32();
        camera->target_y = reader.readI32();
        camera->target_z = reader.readI32();

        camera->sequence = reader.readI8();
        camera->index = reader.readI8();

        camera->fov = reader.readU16();
        camera->roll = reader.readU16();
        camera->timer = reader.readU16();
        camera->speed = reader.readU16();
        camera->flags = reader.readU16();

        camera->room_id = reader.readU32();
        return camera;
    }
};

struct AIObject
{
    uint16_t object_id;    // the objectID from the AI object (AI_FOLLOW is 402)
    uint16_t room;
    int32_t x;
    int32_t y;
    int32_t z;
    uint16_t ocb;
    uint16_t flags;        // The trigger flags (button 1-5, first button has value 2)
    int32_t angle;

    static std::unique_ptr<AIObject> read(io::SDLReader& reader)
    {
        std::unique_ptr<AIObject> object{ new AIObject() };
        object->object_id = reader.readU16();
        object->room = reader.readU16();                        // 4

        object->x = reader.readI32();
        object->y = reader.readI32();
        object->z = reader.readI32();                            // 16

        object->ocb = reader.readU16();
        object->flags = reader.readU16();                       // 20
        object->angle = reader.readI32();                        // 24
        return object;
    }
};

struct CinematicFrame
{
    int16_t roty;        // rotation about Y axis, +/- 32767 == +/- 180 degrees
    int16_t rotz;        // rotation about Z axis, +/- 32767 == +/- 180 degrees
    int16_t rotz2;       // seems to work a lot like rotZ;  I haven't yet been able to
    // differentiate them
    int16_t posz;        // camera position relative to something (target? Lara? room
    // origin?).  pos* are _not_ in world coordinates.
    int16_t posy;        // camera position relative to something (see posZ)
    int16_t posx;        // camera position relative to something (see posZ)
    int16_t unknown;     // changing this can cause a runtime error
    int16_t rotx;        // rotation about X axis, +/- 32767 == +/- 180 degrees

    /// \brief reads a cinematic frame
    static std::unique_ptr<CinematicFrame> read(io::SDLReader& reader)
    {
        std::unique_ptr<CinematicFrame> cf{ new CinematicFrame() };
        cf->roty = reader.readI16();         // rotation about Y axis, +/- 32767 == +/- 180 degrees
        cf->rotz = reader.readI16();         // rotation about Z axis, +/- 32767 == +/- 180 degrees
        cf->rotz2 = reader.readI16();        // seems to work a lot like rotZ;  I haven't yet been able to
                                           // differentiate them
        cf->posz = reader.readI16();         // camera position relative to something (target? Lara? room
                                           // origin?).  pos* are _not_ in world coordinates.
        cf->posy = reader.readI16();         // camera position relative to something (see posZ)
        cf->posx = reader.readI16();         // camera position relative to something (see posZ)
        cf->unknown = reader.readI16();      // changing this can cause a runtime error
        cf->rotx = reader.readI16();         // rotation about X axis, +/- 32767 == +/- 180 degrees
        return cf;
    }
};

struct LightMap
{
    std::array<uint8_t, 32 * 256> map;

    /// \brief reads the lightmap.
    static std::unique_ptr<LightMap> read(io::SDLReader& reader)
    {
        std::unique_ptr<LightMap> lightmap{ new LightMap() };
        reader.readBytes(lightmap->map.data(), lightmap->map.size());
        return lightmap;
    }
};

struct Palette
{
    ByteColor color[256];

    /// \brief reads the 256 colour palette values.
    static std::unique_ptr<Palette> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<Palette> palette{ new Palette() };
        for(int i = 0; i < 256; i++)
            palette->color[i] = ByteColor::readTr1(reader);
        return palette;
    }

    static std::unique_ptr<Palette> readTr2(io::SDLReader& reader)
    {
        std::unique_ptr<Palette> palette{ new Palette() };
        for(int i = 0; i < 256; i++)
            palette->color[i] = ByteColor::readTr2(reader);
        return palette;
    }
};

using FloorData = std::vector<uint16_t>;
} // namespace loader
