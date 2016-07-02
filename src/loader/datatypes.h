#pragma once

#include "io/sdlreader.h"
#include "util/helpers.h"
#include "core/angle.h"
#include "core/coordinates.h"
#include "floordata.h"
#include "color.h"
#include "primitives.h"
#include "meshes.h"
#include "texture.h"
#include "audio.h"

#include <irrlicht.h>
#include <gsl.h>

#include <array>
#include <stdexcept>
#include <vector>
#include <map>
#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <boost/optional.hpp>

/**
 * @defgroup native Native data interface
 *
 * Contains structs and constants directly interfacing the native
 * TR level data or engine internals.
 */

namespace engine
{
    class ItemController;
}

namespace level
{
    class Level;
}

namespace loader
{
    constexpr const uint16_t TextureIndexMaskTr4 = 0x7FFF; // in some custom levels we need to use 0x7FFF flag
    constexpr const uint16_t TextureIndexMask = 0x0FFF;
    //constexpr const uint16_t TR_TEXTURE_SHAPE_MASK = 0x7000;          // still not used
    constexpr const uint16_t TextureFlippedMask = 0x8000;

    constexpr int SectorSize = 1024;
    constexpr int QuarterSectorSize = SectorSize / 4;
    constexpr int HeightLimit = 127 * QuarterSectorSize;

    struct Portal
    {
        uint16_t adjoining_room; ///< \brief which room this portal leads to.
        core::TRCoordinates normal; /**< \brief which way the portal faces.
                                   * the normal points away from the adjacent room->
                                   * to be seen through, it must point toward the viewpoint.
                                   */
        core::TRCoordinates vertices[4]; /**< \brief the corners of this portal.
                                   * the right-hand rule applies with respect to the normal.
                                   * if the right-hand-rule is not followed, the portal will
                                   * contain visual artifacts instead of a viewport to
                                   * Adjoiningroom->
                                   */

        static Portal read(io::SDLReader& reader, const core::TRCoordinates& offset)
        {
            Portal portal;
            portal.adjoining_room = reader.readU16();
            portal.normal = readCoordinates16(reader);
            portal.vertices[0] = readCoordinates16(reader) + offset;
            portal.vertices[1] = readCoordinates16(reader) + offset;
            portal.vertices[2] = readCoordinates16(reader) + offset;
            portal.vertices[3] = readCoordinates16(reader) + offset;
            if( util::fuzzyOne(portal.normal.X) && util::fuzzyZero(portal.normal.Y) && util::fuzzyZero(portal.normal.Z) )
                return portal;
            if( util::fuzzyOne(-portal.normal.X) && util::fuzzyZero(portal.normal.Y) && util::fuzzyZero(portal.normal.Z) )
                return portal;
            if( util::fuzzyZero(portal.normal.X) && util::fuzzyOne(portal.normal.Y) && util::fuzzyZero(portal.normal.Z) )
                return portal;
            if( util::fuzzyZero(portal.normal.X) && util::fuzzyOne(-portal.normal.Y) && util::fuzzyZero(portal.normal.Z) )
                return portal;
            if( util::fuzzyZero(portal.normal.X) && util::fuzzyZero(portal.normal.Y) && util::fuzzyOne(portal.normal.Z) )
                return portal;
            if( util::fuzzyZero(portal.normal.X) && util::fuzzyZero(portal.normal.Y) && util::fuzzyOne(-portal.normal.Z) )
                return portal;
            // std::cerr << "read_tr_room_portal: normal not on world axis");
            return portal;
        }
    };

    struct Sector
    {
        /**
         * @brief Index into FloorData[]
         *
         * @note If this is 0, no floor data is attached to this sector.
         */
        uint16_t floorDataIndex;

        uint16_t boxIndex; //!< Index into Boxes[]/Zones[] (-1 if none)
        uint8_t roomBelow; //!< The number of the room below this one (255 if none)
        int8_t floorHeight; //!< Absolute height of floor (multiply by 256 for world coordinates)
        uint8_t roomAbove; //!< The number of the room above this one (255 if none)
        int8_t ceilingHeight; //!< Absolute height of ceiling (multiply by 256 for world coordinates)

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

        boost::optional<uint8_t> getPortalTarget(const FloorData& floorData) const
        {
            if( floorDataIndex == 0 )
                return {};

            BOOST_ASSERT(floorDataIndex < floorData.size());
            const FloorData::value_type* fdData = &floorData[floorDataIndex];
            BOOST_ASSERT(fdData+1 <= &floorData.back());
            if( extractFDFunction(fdData[0]) == FDFunction::FloorSlant )
            {
                if( isLastFloordataEntry(fdData[0]) )
                    return {};
                fdData += 2;
            }
            BOOST_ASSERT(fdData+1 <= &floorData.back());
            if( extractFDFunction(fdData[0]) == FDFunction::CeilingSlant )
            {
                if( isLastFloordataEntry(fdData[0]) )
                    return {};
                fdData += 2;
            }
            BOOST_ASSERT(fdData+1 <= &floorData.back());
            if( extractFDFunction(fdData[0]) == FDFunction::PortalSector )
            {
                return gsl::narrow_cast<uint8_t>(fdData[1]);
            }

            return {};
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
        irr::scene::ILightSceneNode* node = nullptr;

        core::TRCoordinates position; // world coords
        ByteColor color; // three bytes rgb values
        float intensity; // Calculated intensity
        int16_t specularIntensity; // Light intensity
        uint16_t intensity2; // Almost always equal to Intensity1 [absent from TR1 data files]
        uint32_t specularFade; // Falloff value 1
        uint32_t fade2; // Falloff value 2 [absent from TR1 data files]
        uint8_t light_type; // same as D3D (i.e. 2 is for spotlight)
        uint8_t unknown; // always 0xff?
        int r_inner;
        int r_outer;
        int length;
        int cutoff;
        core::TRCoordinates dir; // direction
        core::TRCoordinates pos2; // world coords
        core::TRCoordinates dir2; // direction

        LightType getLightType() const
        {
            switch( light_type )
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
            light.position = readCoordinates32(reader);
            // read and make consistent
            light.specularIntensity = reader.readI16();
            light.specularFade = reader.readU32();
            // only in TR2
            light.intensity2 = light.specularIntensity;

            light.intensity = irr::core::clamp(light.specularIntensity / 4095.0f, 0.0f, 1.0f);

            light.fade2 = light.specularFade;

            light.r_outer = light.specularFade;
            light.r_inner = light.specularFade / 2;

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
            light.position = readCoordinates32(reader);
            light.specularIntensity = reader.readU16();
            light.intensity2 = reader.readU16();
            light.specularFade = reader.readU32();
            light.fade2 = reader.readU32();

            light.intensity = light.specularIntensity;
            light.intensity /= 4096.0f;

            if( light.intensity > 1.0f )
                light.intensity = 1.0f;

            light.r_outer = light.specularFade;
            light.r_inner = light.specularFade / 2;

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
            light.position = readCoordinates32(reader);
            light.color.r = reader.readU8();
            light.color.g = reader.readU8();
            light.color.b = reader.readU8();
            light.color.a = reader.readU8();
            light.specularFade = reader.readU32();
            light.fade2 = reader.readU32();

            light.intensity = 1.0f;

            light.r_outer = light.specularFade;
            light.r_inner = light.specularFade / 2;

            light.light_type = 1; // Point light
            return light;
        }

        static Light readTr4(io::SDLReader& reader)
        {
            Light light;
            light.position = readCoordinates32(reader);
            light.color = ByteColor::readTr1(reader);
            light.light_type = reader.readU8();
            light.unknown = reader.readU8();
            light.specularIntensity = reader.readU8();
            light.intensity = light.specularIntensity;
            light.intensity /= 32;
            light.r_inner = gsl::narrow<int>(reader.readF());
            light.r_outer = gsl::narrow<int>(reader.readF());
            light.length = gsl::narrow<int>(reader.readF());
            light.cutoff = gsl::narrow<int>(reader.readF());
            light.dir = readCoordinatesF(reader);
            return light;
        }

        static Light readTr5(io::SDLReader& reader)
        {
            Light light;
            light.position = readCoordinatesF(reader);
            //read_tr_colour(src, light.color);
            light.color.r = gsl::narrow<uint8_t>(reader.readF() * 255); // r
            light.color.g = gsl::narrow<uint8_t>(reader.readF() * 255); // g
            light.color.b = gsl::narrow<uint8_t>(reader.readF() * 255); // b
            light.color.a = gsl::narrow<uint8_t>(reader.readF() * 255); // a
            light.intensity = 1.0f;
            /*
            if ((temp != 0) && (temp != 0xCDCDCDCD))
            BOOST_THROW_EXCEPTION( TR_ReadError("read_tr5_room_light: seperator1 has wrong value") );
            */
            light.r_inner = gsl::narrow<int>(reader.readF());
            light.r_outer = gsl::narrow<int>(reader.readF());
            reader.readF(); // rad_input
            reader.readF(); // rad_output
            reader.readF(); // range
            light.dir = readCoordinatesF(reader);
            light.pos2 = readCoordinates32(reader);
            light.dir2 = readCoordinates32(reader);
            light.light_type = reader.readU8();

            auto temp = reader.readU8();
            if( temp != 0xCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room Light: seperator2 has wrong value";

            temp = reader.readU8();
            if( temp != 0xCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room Light: seperator3 has wrong value";

            temp = reader.readU8();
            if( temp != 0xCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room Light: seperator4 has wrong value";

            return light;
        }
    };

    struct Sprite
    {
        uint16_t vertex; // offset into vertex list
        uint16_t texture; // offset into sprite texture list

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
            if( reader.readU16() != 0 )
            BOOST_LOG_TRIVIAL(warning) << "Room Layer: filler2 has wrong value";

            layer.bounding_box_x1 = reader.readF();
            layer.bounding_box_y1 = -reader.readF();
            layer.bounding_box_z1 = -reader.readF();
            layer.bounding_box_x2 = reader.readF();
            layer.bounding_box_y2 = -reader.readF();
            layer.bounding_box_z2 = -reader.readF();
            if( reader.readU32() != 0 )
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
        core::TRCoordinates vertex; // where this vertex lies (relative to tr2_room_info::x/z)
        int16_t lighting1;
        uint16_t attributes; // A set of flags for special rendering effects [absent from TR1 data files]
        // 0x8000 something to do with water surface
        // 0x4000 under water lighting modulation and
        // movement if viewed from above water surface
        // 0x2000 water/quicksand surface movement
        // 0x0010 "normal"
        int16_t lighting2; // Almost always equal to Lighting1 [absent from TR1 data files]
        // TR5 -->
        core::TRCoordinates normal;
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
            room_vertex.vertex = readCoordinates16(reader);
            // read and make consistent
            int tmp = reader.readU16();
            BOOST_ASSERT(tmp < 8192);
            room_vertex.lighting1 = (32768 - tmp * 4);
            // only in TR2
            room_vertex.lighting2 = room_vertex.lighting1;
            room_vertex.attributes = 0;
            // only in TR5
            room_vertex.normal = {0,0,0};
            auto f = gsl::narrow_cast<irr::u8>(room_vertex.lighting1 / 32768.0f * 255);
            room_vertex.color.set(255, f, f, f);
            return room_vertex;
        }

        static RoomVertex readTr2(io::SDLReader& reader)
        {
            RoomVertex room_vertex;
            room_vertex.vertex = readCoordinates16(reader);
            // read and make consistent
            room_vertex.lighting1 = (8191 - reader.readI16()) << 2;
            room_vertex.attributes = reader.readU16();
            room_vertex.lighting2 = (8191 - reader.readI16()) << 2;
            // only in TR5
            room_vertex.normal = {0,0,0};
            auto f = gsl::narrow<irr::u8>(room_vertex.lighting2 / 32768.0f * 255);
            room_vertex.color.set(255, f, f, f);
            return room_vertex;
        }

        static RoomVertex readTr3(io::SDLReader& reader)
        {
            RoomVertex room_vertex;
            room_vertex.vertex = readCoordinates16(reader);
            // read and make consistent
            room_vertex.lighting1 = reader.readI16();
            room_vertex.attributes = reader.readU16();
            room_vertex.lighting2 = reader.readI16();
            // only in TR5
            room_vertex.normal = {0,0,0};
            room_vertex.color.set(255,
                                  gsl::narrow<irr::u8>(((room_vertex.lighting2 & 0x7C00) >> 10) / 62.0f * 255),
                                  gsl::narrow<irr::u8>(((room_vertex.lighting2 & 0x03E0) >> 5) / 62.0f * 255),
                                  gsl::narrow<irr::u8>((room_vertex.lighting2 & 0x001F) / 62.0f * 255));
            return room_vertex;
        }

        static RoomVertex readTr4(io::SDLReader& reader)
        {
            RoomVertex room_vertex;
            room_vertex.vertex = readCoordinates16(reader);
            // read and make consistent
            room_vertex.lighting1 = reader.readI16();
            room_vertex.attributes = reader.readU16();
            room_vertex.lighting2 = reader.readI16();
            // only in TR5
            room_vertex.normal = {0,0,0};

            room_vertex.color.set(255,
                                  gsl::narrow<irr::u8>(((room_vertex.lighting2 & 0x7C00) >> 10) / 31.0f * 255),
                                  gsl::narrow<irr::u8>(((room_vertex.lighting2 & 0x03E0) >> 5) / 31.0f * 255),
                                  gsl::narrow<irr::u8>((room_vertex.lighting2 & 0x001F) / 31.0f * 255));
            return room_vertex;
        }

        static RoomVertex readTr5(io::SDLReader& reader)
        {
            RoomVertex vert;
            vert.vertex = readCoordinatesF(reader);
            vert.normal = readCoordinatesF(reader);
            auto b = reader.readU8();
            auto g = reader.readU8();
            auto r = reader.readU8();
            auto a = reader.readU8();
            vert.color.set(a, r, g, b);
            return vert;
        }
    };

    struct Room
    {
        irr::scene::ISceneNode* node = nullptr;

        // Various room flags specify various room options. Mostly, they
        // specify environment type and some additional actions which should
        // be performed in such rooms.
        static constexpr uint16_t TR_ROOM_FLAG_WATER = 0x0001;
        static constexpr uint16_t TR_ROOM_FLAG_QUICKSAND = 0x0002; // Moved from 0x0080 to avoid confusion with NL.
        static constexpr uint16_t TR_ROOM_FLAG_SKYBOX = 0x0008;
        static constexpr uint16_t TR_ROOM_FLAG_UNKNOWN1 = 0x0010;
        static constexpr uint16_t TR_ROOM_FLAG_WIND = 0x0020;
        static constexpr uint16_t TR_ROOM_FLAG_UNKNOWN2 = 0x0040; ///< @FIXME: Find what it means!!! Always set by Dxtre3d.
        static constexpr uint16_t TR_ROOM_FLAG_NO_LENSFLARE = 0x0080; // In TR4-5. Was quicksand in TR3.
        static constexpr uint16_t TR_ROOM_FLAG_MIST = 0x0100; ///< @FIXME: Unknown meaning in TR1!!!
        static constexpr uint16_t TR_ROOM_FLAG_CAUSTICS = 0x0200;
        static constexpr uint16_t TR_ROOM_FLAG_UNKNOWN3 = 0x0400;
        static constexpr uint16_t TR_ROOM_FLAG_DAMAGE = 0x0800; ///< @FIXME: Is it really damage (D)?
        static constexpr uint16_t TR_ROOM_FLAG_POISON = 0x1000; ///< @FIXME: Is it really poison (P)?

        core::TRCoordinates position;
        long lowestHeight;
        long greatestHeight;
        std::vector<Layer> layers;
        std::vector<RoomVertex> vertices;
        std::vector<QuadFace> rectangles;
        std::vector<Triangle> triangles;
        std::vector<Sprite> sprites;
        std::vector<Portal> portals;
        uint16_t sectorCountZ; // "width" of sector list
        uint16_t sectorCountX; // "height" of sector list
        std::vector<Sector> sectors; // [NumXsectors * NumZsectors] list of sectors in this room
        int16_t darkness; // This and the next one only affect externally-lit objects
        int16_t intensity2; // Almost always the same value as AmbientIntensity1 [absent from TR1 data files]
        int16_t lightMode; // (present only in TR2: 0 is normal, 1 is flickering(?), 2 and 3 are uncertain)
        std::vector<Light> lights; // [NumLights] list of point lights
        std::vector<RoomStaticMesh> staticMeshes; // [NumStaticMeshes]list of static meshes
        int16_t alternateRoom; // number of the room that this room can alternate
        int8_t alternateGroup; // number of group which is used to switch alternate rooms
        // with (e.g. empty/filled with water is implemented as an empty room that alternates with a full room)

        uint16_t flags;

        // Flag bits:
        // 0x0001 - room is filled with water,
        // 0x0020 - Lara's ponytail gets blown by the wind;
        // TR1 has only the water flag and the extra unknown flag 0x0100.
        // TR3 most likely has flags for "is raining", "is snowing", "water is cold", and "is
        // filled by quicksand", among others.

        inline bool isWaterRoom() const noexcept
        {
            return (flags & TR_ROOM_FLAG_WATER) != 0;
        }

        uint8_t waterScheme;
        // Water scheme is used with various room options, for example, R and M room flags in TRLE.
        // Also, it specifies lighting scheme, when 0x4000 vertex attribute is set.

        ReverbType reverbInfo;

        // Reverb info is used in TR3-5 and contains index that specifies reverb type.
        // 0 - Outside, 1 - Small room, 2 - Medium room, 3 - Large room, 4 - Pipe.

        FloatColor lightColor; // Present in TR5 only

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
            std::unique_ptr<Room> room{new Room()};

            // read and change coordinate system
            room->position.X = reader.readI32();
            room->position.Y = 0;
            room->position.Z = reader.readI32();
            room->lowestHeight = -reader.readI32();
            room->greatestHeight = -reader.readI32();

            auto num_data_words = reader.readU32();

            auto position = reader.tell();

            reader.readVector(room->vertices, reader.readU16(), &RoomVertex::readTr1);
            reader.readVector(room->rectangles, reader.readU16(), &QuadFace::readTr1);
            reader.readVector(room->triangles, reader.readU16(), &Triangle::readTr1);
            reader.readVector(room->sprites, reader.readU16(), &Sprite::read);

            // set to the right position in case that there is some unused data
            reader.seek(position + num_data_words * 2);

            room->portals.resize(reader.readU16());
            for( auto& p : room->portals )
                p = Portal::read(reader, room->position);

            room->sectorCountZ = reader.readU16();
            room->sectorCountX = reader.readU16();
            reader.readVector(room->sectors, room->sectorCountZ * room->sectorCountX, &Sector::read);

            // read and make consistent
            room->darkness = reader.readI16();
            // only in TR2-TR4
            room->intensity2 = room->darkness;
            // only in TR2
            room->lightMode = 0;

            reader.readVector(room->lights, reader.readU16(), &Light::readTr1);
            reader.readVector(room->staticMeshes, reader.readU16(), &RoomStaticMesh::readTr1);

            room->alternateRoom = reader.readI16();
            room->alternateGroup = 0; // Doesn't exist in TR1-3

            room->flags = reader.readU16();
            room->reverbInfo = ReverbType::MediumRoom;

            room->lightColor.r = 1.0f;
            room->lightColor.g = 1.0f;
            room->lightColor.b = 1.0f;
            room->lightColor.a = 1.0f;
            return room;
        }

        static std::unique_ptr<Room> readTr2(io::SDLReader& reader)
        {
            std::unique_ptr<Room> room{new Room()};
            // read and change coordinate system
            room->position.X = reader.readI32();
            room->position.Y = 0;
            room->position.Z = reader.readI32();
            room->lowestHeight = reader.readI32();
            room->greatestHeight = reader.readI32();

            auto num_data_words = reader.readU32();

            auto position = reader.tell();

            reader.readVector(room->vertices, reader.readU16(), &RoomVertex::readTr2);
            reader.readVector(room->rectangles, reader.readU16(), &QuadFace::readTr1);
            reader.readVector(room->triangles, reader.readU16(), &Triangle::readTr1);
            reader.readVector(room->sprites, reader.readU16(), &Sprite::read);

            // set to the right position in case that there is some unused data
            reader.seek(position + num_data_words * 2);

            room->portals.resize(reader.readU16());
            for( size_t i = 0; i < room->portals.size(); i++ )
                room->portals[i] = Portal::read(reader, room->position);

            room->sectorCountZ = reader.readU16();
            room->sectorCountX = reader.readU16();
            reader.readVector(room->sectors, room->sectorCountZ * room->sectorCountX, &Sector::read);

            // read and make consistent
            room->darkness = (8191 - reader.readI16()) << 2;
            room->intensity2 = (8191 - reader.readI16()) << 2;
            room->lightMode = reader.readI16();

            reader.readVector(room->lights, reader.readU16(), &Light::readTr2);
            reader.readVector(room->staticMeshes, reader.readU16(), &RoomStaticMesh::readTr2);

            room->alternateRoom = reader.readI16();
            room->alternateGroup = 0; // Doesn't exist in TR1-3

            room->flags = reader.readU16();

            if( room->flags & 0x0020 )
            {
                room->reverbInfo = ReverbType::Outside;
            }
            else
            {
                room->reverbInfo = ReverbType::MediumRoom;
            }

            room->lightColor.r = room->darkness / 16384.0f;
            room->lightColor.g = room->darkness / 16384.0f;
            room->lightColor.b = room->darkness / 16384.0f;
            room->lightColor.a = 1.0f;
            return room;
        }

        static std::unique_ptr<Room> readTr3(io::SDLReader& reader)
        {
            std::unique_ptr<Room> room{new Room()};

            // read and change coordinate system
            room->position.X = reader.readI32();
            room->position.Y = 0;
            room->position.Z = reader.readI32();
            room->lowestHeight = reader.readI32();
            room->greatestHeight = reader.readI32();

            auto num_data_words = reader.readU32();

            auto position = reader.tell();

            reader.readVector(room->vertices, reader.readU16(), &RoomVertex::readTr3);
            reader.readVector(room->rectangles, reader.readU16(), &QuadFace::readTr1);
            reader.readVector(room->triangles, reader.readU16(), &Triangle::readTr1);
            reader.readVector(room->sprites, reader.readU16(), &Sprite::read);

            // set to the right position in case that there is some unused data
            reader.seek(position + num_data_words * 2);

            room->portals.resize(reader.readU16());
            for( size_t i = 0; i < room->portals.size(); i++ )
                room->portals[i] = Portal::read(reader, room->position);

            room->sectorCountZ = reader.readU16();
            room->sectorCountX = reader.readU16();
            reader.readVector(room->sectors, room->sectorCountZ * room->sectorCountX, &Sector::read);

            room->darkness = reader.readI16();
            room->intensity2 = reader.readI16();

            // only in TR2
            room->lightMode = 0;

            reader.readVector(room->lights, reader.readU16(), &Light::readTr3);
            reader.readVector(room->staticMeshes, reader.readU16(), &RoomStaticMesh::readTr3);

            room->alternateRoom = reader.readI16();
            room->alternateGroup = 0; // Doesn't exist in TR1-3

            room->flags = reader.readU16();

            if( room->flags & 0x0080 )
            {
                room->flags |= 0x0002; // Move quicksand flag to another bit to avoid confusion with NL flag.
                room->flags ^= 0x0080;
            }

            // Only in TR3-5

            room->waterScheme = reader.readU8();
            room->reverbInfo = static_cast<ReverbType>(reader.readU8());

            reader.skip(1); // Alternate_group override?

            room->lightColor.r = room->darkness / 65534.0f;
            room->lightColor.g = room->darkness / 65534.0f;
            room->lightColor.b = room->darkness / 65534.0f;
            room->lightColor.a = 1.0f;
            return room;
        }

        static std::unique_ptr<Room> readTr4(io::SDLReader& reader)
        {
            std::unique_ptr<Room> room{new Room()};
            // read and change coordinate system
            room->position.X = reader.readI32();
            room->position.Y = 0;
            room->position.Z = reader.readI32();
            room->lowestHeight = reader.readI32();
            room->greatestHeight = reader.readI32();

            auto num_data_words = reader.readU32();

            auto position = reader.tell();

            reader.readVector(room->vertices, reader.readU16(), &RoomVertex::readTr4);
            reader.readVector(room->rectangles, reader.readU16(), &QuadFace::readTr1);
            reader.readVector(room->triangles, reader.readU16(), &Triangle::readTr1);
            reader.readVector(room->sprites, reader.readU16(), &Sprite::read);

            // set to the right position in case that there is some unused data
            reader.seek(position + num_data_words * 2);

            room->portals.resize(reader.readU16());
            for( size_t i = 0; i < room->portals.size(); i++ )
                room->portals[i] = Portal::read(reader, room->position);

            room->sectorCountZ = reader.readU16();
            room->sectorCountX = reader.readU16();
            reader.readVector(room->sectors, room->sectorCountZ * room->sectorCountX, &Sector::read);

            room->darkness = reader.readI16();
            room->intensity2 = reader.readI16();

            // only in TR2
            room->lightMode = 0;

            reader.readVector(room->lights, reader.readU16(), &Light::readTr4);
            reader.readVector(room->staticMeshes, reader.readU16(), &RoomStaticMesh::readTr4);

            room->alternateRoom = reader.readI16();
            room->flags = reader.readU16();

            // Only in TR3-5

            room->waterScheme = reader.readU8();
            room->reverbInfo = static_cast<ReverbType>(reader.readU8());

            // Only in TR4-5

            room->alternateGroup = reader.readU8();

            room->lightColor.r = (room->intensity2 & 0x00FF) / 255.0f;
            room->lightColor.g = ((room->darkness & 0xFF00) >> 8) / 255.0f;
            room->lightColor.b = (room->darkness & 0x00FF) / 255.0f;
            room->lightColor.a = ((room->intensity2 & 0xFF00) >> 8) / 255.0f;
            return room;
        }

        static std::unique_ptr<Room> readTr5(io::SDLReader& reader)
        {
            if( reader.readU32() != 0x414C4558 )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: 'XELA' not found";

            const auto room_data_size = reader.readU32();
            const auto position = reader.tell();
            const auto endPos = position + room_data_size;

            std::unique_ptr<Room> room{new Room()};
            room->darkness = 32767;
            room->intensity2 = 32767;
            room->lightMode = 0;

            if( reader.readU32() != 0xCDCDCDCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator1 has wrong value";

            /*portal_offset = */
            reader.readI32(); // StartPortalOffset?   // endSDOffset
            auto sector_data_offset = reader.readU32(); // StartSDOffset
            auto temp = reader.readU32();
            if( temp != 0 && temp != 0xCDCDCDCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator2 has wrong value";

            auto static_meshes_offset = reader.readU32(); // endPortalOffset
            // static_meshes_offset or room_layer_offset
            // read and change coordinate system
            room->position.X = reader.readI32();
            room->position.Y = reader.readU32();
            room->position.Z = reader.readI32();
            room->lowestHeight = reader.readI32();
            room->greatestHeight = reader.readI32();

            room->sectorCountZ = reader.readU16();
            room->sectorCountX = reader.readU16();

            room->lightColor.b = reader.readU8() / 255.0f;
            room->lightColor.g = reader.readU8() / 255.0f;
            room->lightColor.r = reader.readU8() / 255.0f;
            room->lightColor.a = reader.readU8() / 255.0f;
            //room->light_colour.a = 1.0f;

            room->lights.resize(reader.readU16());
            if( room->lights.size() > 512 )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: lights.size() > 512";

            room->staticMeshes.resize(reader.readU16());
            if( room->staticMeshes.size() > 512 )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: static_meshes.size() > 512";

            room->reverbInfo = static_cast<ReverbType>(reader.readU8());
            room->alternateGroup = reader.readU8();
            room->waterScheme = gsl::narrow<uint8_t>(reader.readU16());

            if( reader.readU32() != 0x00007FFF )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: filler1 has wrong value";

            if( reader.readU32() != 0x00007FFF )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: filler2 has wrong value";

            if( reader.readU32() != 0xCDCDCDCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator4 has wrong value";

            if( reader.readU32() != 0xCDCDCDCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator5 has wrong value";

            if( reader.readU32() != 0xFFFFFFFF )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator6 has wrong value";

            room->alternateRoom = reader.readI16();

            room->flags = reader.readU16();

            room->unknown_r1 = reader.readU32();
            room->unknown_r2 = reader.readU32();
            room->unknown_r3 = reader.readU32();

            temp = reader.readU32();
            if( temp != 0 && temp != 0xCDCDCDCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator7 has wrong value";

            room->unknown_r4a = reader.readU16();
            room->unknown_r4b = reader.readU16();

            room->room_x = reader.readF();
            room->unknown_r5 = reader.readU32();
            room->room_z = -reader.readF();

            if( reader.readU32() != 0xCDCDCDCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator8 has wrong value";

            if( reader.readU32() != 0xCDCDCDCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator9 has wrong value";

            if( reader.readU32() != 0xCDCDCDCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator10 has wrong value";

            if( reader.readU32() != 0xCDCDCDCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator11 has wrong value";

            temp = reader.readU32();
            if( temp != 0 && temp != 0xCDCDCDCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator12 has wrong value";

            if( reader.readU32() != 0xCDCDCDCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator13 has wrong value";

            auto num_triangles = reader.readU32();
            if( num_triangles == 0xCDCDCDCD )
                num_triangles = 0;
            if( num_triangles > 512 )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: triangles.size() > 512";
            room->triangles.resize(num_triangles);

            auto num_rectangles = reader.readU32();
            if( num_rectangles == 0xCDCDCDCD )
                num_rectangles = 0;
            if( num_rectangles > 1024 )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: rectangles.size() > 1024";
            room->rectangles.resize(num_rectangles);

            if( reader.readU32() != 0 )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator14 has wrong value";

            /*light_size = */
            reader.readU32();
            auto numL2 = reader.readU32();
            if( numL2 != room->lights.size() )
            BOOST_THROW_EXCEPTION(std::runtime_error("TR5 Room: numLights2 != lights.size()"));

            room->unknown_r6 = reader.readU32();
            room->room_y_top = -reader.readF();
            room->room_y_bottom = -reader.readF();

            room->layers.resize(reader.readU32());

            auto layer_offset = reader.readU32();
            auto vertices_offset = reader.readU32();
            auto poly_offset = reader.readU32();
            auto poly_offset2 = reader.readU32();
            if( poly_offset != poly_offset2 )
            BOOST_THROW_EXCEPTION(std::runtime_error("TR5 Room: poly_offset != poly_offset2"));

            auto vertices_size = reader.readU32();
            if( vertices_size % 28 != 0 )
            BOOST_THROW_EXCEPTION(std::runtime_error("TR5 Room: vertices_size has wrong value"));

            if( reader.readU32() != 0xCDCDCDCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator15 has wrong value";

            if( reader.readU32() != 0xCDCDCDCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator16 has wrong value";

            if( reader.readU32() != 0xCDCDCDCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator17 has wrong value";

            if( reader.readU32() != 0xCDCDCDCD )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator18 has wrong value";

            for( size_t i = 0; i < room->lights.size(); i++ )
                room->lights[i] = Light::readTr5(reader);

            reader.seek(position + 208 + sector_data_offset);

            reader.readVector(room->sectors, room->sectorCountZ * room->sectorCountX, &Sector::read);

            room->portals.resize(reader.readI16());
            for( size_t i = 0; i < room->portals.size(); i++ )
                room->portals[i] = Portal::read(reader, room->position);

            reader.seek(position + 208 + static_meshes_offset);

            for( size_t i = 0; i < room->staticMeshes.size(); i++ )
                room->staticMeshes[i] = RoomStaticMesh::readTr4(reader);

            reader.seek(position + 208 + layer_offset);

            for( size_t i = 0; i < room->layers.size(); i++ )
                room->layers[i] = Layer::read(reader);

            reader.seek(position + 208 + poly_offset);

            {
                uint32_t vertex_index = 0;
                uint32_t rectangle_index = 0;
                uint32_t triangle_index = 0;

                for( size_t i = 0; i < room->layers.size(); i++ )
                {
                    uint32_t j;

                    for( j = 0; j < room->layers[i].num_rectangles; j++ )
                    {
                        room->rectangles[rectangle_index] = QuadFace::readTr4(reader);
                        room->rectangles[rectangle_index].vertices[0] += vertex_index;
                        room->rectangles[rectangle_index].vertices[1] += vertex_index;
                        room->rectangles[rectangle_index].vertices[2] += vertex_index;
                        room->rectangles[rectangle_index].vertices[3] += vertex_index;
                        rectangle_index++;
                    }
                    for( j = 0; j < room->layers[i].num_triangles; j++ )
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
                for( size_t i = 0; i < room->layers.size(); i++ )
                {
                    for( uint16_t j = 0; j < room->layers[i].num_vertices; j++ )
                        room->vertices[vertex_index++] = RoomVertex::readTr5(reader);
                }
            }

            reader.seek(endPos);

            return room;
        }

        irr::scene::IMeshSceneNode* createSceneNode(irr::scene::ISceneManager* mgr, int dumpIdx, const level::Level& level, const std::map<TextureLayoutProxy::TextureKey, irr::video::SMaterial>& materials, const std::vector<irr::video::ITexture*>& textures, const std::vector<irr::scene::SMesh*>& staticMeshes, render::TextureAnimator& animator);

        const Sector* getSectorByAbsolutePosition(core::TRCoordinates position) const
        {
            position -= this->position;
            return getSectorByIndex(position.X / SectorSize, position.Z / SectorSize);
        }

        gsl::not_null<const Sector*> findFloorSectorWithClampedPosition(core::TRCoordinates position) const
        {
            position -= this->position;
            return findFloorSectorWithClampedIndex(position.X / SectorSize, position.Z / SectorSize);
        }

        const Sector* getSectorByIndex(int dx, int dz) const
        {
            if( dx < 0 || dx >= sectorCountX )
                return nullptr;
            if( dz < 0 || dz >= sectorCountZ )
                return nullptr;
            return &sectors[sectorCountZ * dx + dz];
        }

        gsl::not_null<const Sector*> findFloorSectorWithClampedIndex(int dx, int dz) const
        {
            if( dz <= 0 )
            {
                dz = 0;
                dx = irr::core::clamp(dx, 1, sectorCountX - 2);
            }
            else if( dz >= sectorCountZ - 1 )
            {
                dz = sectorCountZ - 1;
                dx = irr::core::clamp(dx, 1, sectorCountX - 2);
            }
            else
            {
                dx = irr::core::clamp(dx, 0, sectorCountX - 1);
            }
            return getSectorByIndex(dx, dz);
        }

        static void patchHeightsForBlock(const engine::ItemController& ctrl, int height);
    };

    enum class TimerState
    {
        Active,
        Idle,
        Stopped
    };

    struct SpriteTexture
    {
        uint16_t texture;
        irr::core::vector2df t0;
        irr::core::vector2df t1;

        int16_t left_side;
        int16_t top_side;
        int16_t right_side;
        int16_t bottom_side;

        /** \brief reads sprite texture definition.
          *
          * some sanity checks get done and if they fail an exception gets thrown.
          */
        static std::unique_ptr<SpriteTexture> readTr1(io::SDLReader& reader)
        {
            std::unique_ptr<SpriteTexture> sprite_texture{new SpriteTexture()};

            sprite_texture->texture = reader.readU16();
            if( sprite_texture->texture > 64 )
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
            sprite_texture->t0.X = tx / 255.0f;
            sprite_texture->t0.Y = ty / 255.0f;
            sprite_texture->t1.X = sprite_texture->t0.X + w / 255.0f;
            sprite_texture->t1.Y = sprite_texture->t0.Y + h / 255.0f;

            sprite_texture->left_side = tleft;
            sprite_texture->right_side = tright;
            sprite_texture->top_side = -tbottom;
            sprite_texture->bottom_side = -ttop;
            return sprite_texture;
        }

        static std::unique_ptr<SpriteTexture> readTr4(io::SDLReader& reader)
        {
            std::unique_ptr<SpriteTexture> sprite_texture{new SpriteTexture()};
            sprite_texture->texture = reader.readU16();
            if( sprite_texture->texture > 128 )
            BOOST_LOG_TRIVIAL(warning) << "TR4 Sprite Texture: tile > 128";

            int tx = reader.readU8();
            int ty = reader.readU8();
            int tw = reader.readU16();
            int th = reader.readU16();
            int tleft = reader.readI16();
            int ttop = reader.readI16();
            int tright = reader.readI16();
            int tbottom = reader.readI16();

            sprite_texture->t0.X = tleft / 255.0f;
            sprite_texture->t0.Y = tright / 255.0f;
            sprite_texture->t1.X = tbottom / 255.0f;
            sprite_texture->t1.Y = ttop / 255.0f;

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
        uint32_t type; // Item identifier (matched in Items[])
        int16_t length; // negative of "how many sprites are in this sequence"
        int16_t offset; // where (in sprite texture list) this sequence starts

        /** \brief reads sprite sequence definition.
          *
          * length is negative when read and thus gets negated.
          */
        static std::unique_ptr<SpriteSequence> read(io::SDLReader& reader)
        {
            std::unique_ptr<SpriteSequence> sprite_sequence{new SpriteSequence()};
            sprite_sequence->type = reader.readU32();
            sprite_sequence->length = -reader.readI16();
            sprite_sequence->offset = reader.readI16();
            return sprite_sequence;
        }
    };

    struct Box
    {
        int32_t zmin; // sectors (* 1024 units)
        int32_t zmax;
        int32_t xmin;
        int32_t xmax;
        int16_t true_floor; // Y value (no scaling)
        uint16_t overlap_index; // index into Overlaps[]. The high bit is sometimes set; this
        // occurs in front of swinging doors and the like.

        static std::unique_ptr<Box> readTr1(io::SDLReader& reader)
        {
            std::unique_ptr<Box> box{new Box()};
            box->zmin = reader.readI32();
            box->zmax = reader.readI32();
            box->xmin = reader.readI32();
            box->xmax = reader.readI32();
            box->true_floor = reader.readI16();
            box->overlap_index = reader.readU16();
            return box;
        }

        static std::unique_ptr<Box> readTr2(io::SDLReader& reader)
        {
            std::unique_ptr<Box> box{new Box()};
            box->zmin = 1024 * reader.readU8();
            box->zmax = 1024 * reader.readU8();
            box->xmin = 1024 * reader.readU8();
            box->xmax = 1024 * reader.readU8();
            box->true_floor = reader.readI16();
            box->overlap_index = reader.readU16();
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
        static std::unique_ptr<Zone> read(io::SDLReader& reader, size_t n)
        {
            Expects(n == 2 || n == 4);

            std::unique_ptr<Zone> zone{new Zone()};
            zone->flyZoneNormal = reader.readU16();
            for( size_t i = 0; i < n; ++i )
                zone->groundZonesNormal.emplace_back(reader.readU16());
            zone->flyZoneAlternate = reader.readU16();
            for( size_t i = 0; i < n; ++i )
                zone->groundZonesAlternate.emplace_back(reader.readU16());
            return zone;
        }
    };

    struct Camera
    {
        int32_t x;
        int32_t y;
        int32_t z;
        uint16_t room;
        //! @todo mutable flags
        mutable uint16_t flags;

        static std::unique_ptr<Camera> read(io::SDLReader& reader)
        {
            std::unique_ptr<Camera> camera{new Camera()};
            camera->x = reader.readI32();
            camera->y = reader.readI32();
            camera->z = reader.readI32();

            camera->room = reader.readU16();
            camera->flags = reader.readU16();
            return camera;
        }

        constexpr bool isActive() const noexcept
        {
            return (flags & 1) != 0;
        }

        void setActive(bool flg) const noexcept
        {
            if( flg )
                flags |= 1;
            else
                flags &= ~1;
        }
    };

    struct FlybyCamera
    {
        int32_t cam_x;
        int32_t cam_y;
        int32_t cam_z;
        int32_t target_x;
        int32_t target_y;
        int32_t target_z;
        uint8_t sequence;
        uint8_t index;
        uint16_t fov;
        uint16_t roll;
        uint16_t timer;
        uint16_t speed;
        uint16_t flags;
        uint32_t room_id;

        static std::unique_ptr<FlybyCamera> read(io::SDLReader& reader)
        {
            std::unique_ptr<FlybyCamera> camera{new FlybyCamera()};
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
        uint16_t object_id; // the objectID from the AI object (AI_FOLLOW is 402)
        uint16_t room;
        int32_t x;
        int32_t y;
        int32_t z;
        uint16_t ocb;
        uint16_t flags; // The trigger flags (button 1-5, first button has value 2)
        int32_t angle;

        static std::unique_ptr<AIObject> read(io::SDLReader& reader)
        {
            std::unique_ptr<AIObject> object{new AIObject()};
            object->object_id = reader.readU16();
            object->room = reader.readU16(); // 4

            object->x = reader.readI32();
            object->y = reader.readI32();
            object->z = reader.readI32(); // 16

            object->ocb = reader.readU16();
            object->flags = reader.readU16(); // 20
            object->angle = reader.readI32(); // 24
            return object;
        }
    };

    struct CinematicFrame
    {
        int16_t roty; // rotation about Y axis, +/- 32767 == +/- 180 degrees
        int16_t rotz; // rotation about Z axis, +/- 32767 == +/- 180 degrees
        int16_t rotz2; // seems to work a lot like rotZ;  I haven't yet been able to
        // differentiate them
        int16_t posz; // camera position relative to something (target? Lara? room
        // origin?).  pos* are _not_ in world coordinates.
        int16_t posy; // camera position relative to something (see posZ)
        int16_t posx; // camera position relative to something (see posZ)
        int16_t unknown; // changing this can cause a runtime error
        int16_t rotx; // rotation about X axis, +/- 32767 == +/- 180 degrees

        /// \brief reads a cinematic frame
        static std::unique_ptr<CinematicFrame> read(io::SDLReader& reader)
        {
            std::unique_ptr<CinematicFrame> cf{new CinematicFrame()};
            cf->roty = reader.readI16(); // rotation about Y axis, +/- 32767 == +/- 180 degrees
            cf->rotz = reader.readI16(); // rotation about Z axis, +/- 32767 == +/- 180 degrees
            cf->rotz2 = reader.readI16(); // seems to work a lot like rotZ;  I haven't yet been able to
            // differentiate them
            cf->posz = reader.readI16(); // camera position relative to something (target? Lara? room
            // origin?).  pos* are _not_ in world coordinates.
            cf->posy = reader.readI16(); // camera position relative to something (see posZ)
            cf->posx = reader.readI16(); // camera position relative to something (see posZ)
            cf->unknown = reader.readI16(); // changing this can cause a runtime error
            cf->rotx = reader.readI16(); // rotation about X axis, +/- 32767 == +/- 180 degrees
            return cf;
        }
    };

    struct LightMap
    {
        std::array<uint8_t, 32 * 256> map;

        /// \brief reads the lightmap.
        static std::unique_ptr<LightMap> read(io::SDLReader& reader)
        {
            std::unique_ptr<LightMap> lightmap{new LightMap()};
            reader.readBytes(lightmap->map.data(), lightmap->map.size());
            return lightmap;
        }
    };
} // namespace loader
