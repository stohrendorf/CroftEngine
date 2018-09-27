#pragma once

#include "color.h"
#include "texture.h"
#include "util.h"
#include "core/boundingbox.h"

namespace core
{
class Angle;
}

namespace render
{
class TextureAnimator;
}

namespace loader
{
struct RoomStaticMesh
{
    core::TRVec position; // world coords
    int16_t rotation;
    int16_t darkness; // Constant lighting; -1 means use mesh lighting
    int16_t intensity2; // Like Intensity 1, and almost always the same value [absent from TR1 data files]
    uint16_t meshId; // which StaticMesh item to draw
    FloatColor tint; // extracted from intensity

    float getBrightness() const
    {
        return 2.0f - darkness / 8191.0f;
    }

    /** \brief reads a room static mesh definition.
    *
    * rotation gets converted to float and scaled appropriately.
    * darkness gets converted, so it matches the 0-32768 range introduced in TR3.
    * intensity2 is introduced in TR2 and is set to darkness for TR1.
    */
    static RoomStaticMesh readTr1(io::SDLReader& reader)
    {
        RoomStaticMesh room_static_mesh;
        room_static_mesh.position = readCoordinates32( reader );
        room_static_mesh.rotation = reader.readI16();
        room_static_mesh.darkness = reader.readI16();
        room_static_mesh.meshId = reader.readU16();

        // only in TR2
        room_static_mesh.intensity2 = room_static_mesh.darkness;

        room_static_mesh.tint.b = room_static_mesh.tint.g = room_static_mesh.tint.r = room_static_mesh.intensity2
                                                                                      / 16384.0f;
        room_static_mesh.tint.a = 1.0f;
        return room_static_mesh;
    }

    static RoomStaticMesh readTr2(io::SDLReader& reader)
    {
        RoomStaticMesh room_static_mesh;
        room_static_mesh.position = readCoordinates32( reader );
        room_static_mesh.rotation = reader.readI16();
        room_static_mesh.darkness = reader.readI16();
        room_static_mesh.intensity2 = reader.readI16();
        room_static_mesh.meshId = reader.readU16();
        // make consistent
        if( room_static_mesh.darkness >= 0 )
            room_static_mesh.darkness = (8191 - room_static_mesh.darkness) << 2;
        if( room_static_mesh.intensity2 >= 0 )
            room_static_mesh.intensity2 = (8191 - room_static_mesh.intensity2) << 2;

        room_static_mesh.tint.b = room_static_mesh.tint.g = room_static_mesh.tint.r = room_static_mesh.intensity2
                                                                                      / 16384.0f;
        room_static_mesh.tint.a = 1.0f;
        return room_static_mesh;
    }

    static RoomStaticMesh readTr3(io::SDLReader& reader)
    {
        RoomStaticMesh room_static_mesh;
        room_static_mesh.position = readCoordinates32( reader );
        room_static_mesh.rotation = reader.readI16();
        room_static_mesh.darkness = reader.readI16();
        room_static_mesh.intensity2 = reader.readI16();
        room_static_mesh.meshId = reader.readU16();

        room_static_mesh.tint.r = (room_static_mesh.darkness & 0x001F) / 62.0f;

        room_static_mesh.tint.g = ((room_static_mesh.darkness & 0x03E0) >> 5) / 62.0f;

        room_static_mesh.tint.b = ((room_static_mesh.darkness & 0x7C00) >> 10) / 62.0f;
        room_static_mesh.tint.a = 1.0f;
        return room_static_mesh;
    }

    static RoomStaticMesh readTr4(io::SDLReader& reader)
    {
        RoomStaticMesh room_static_mesh;
        room_static_mesh.position = readCoordinates32( reader );
        room_static_mesh.rotation = reader.readI16();
        room_static_mesh.darkness = reader.readI16();
        room_static_mesh.intensity2 = reader.readI16();
        room_static_mesh.meshId = reader.readU16();

        room_static_mesh.tint.r = (room_static_mesh.darkness & 0x001F) / 31.0f;

        room_static_mesh.tint.g = ((room_static_mesh.darkness & 0x03E0) >> 5) / 31.0f;

        room_static_mesh.tint.b = ((room_static_mesh.darkness & 0x7C00) >> 10) / 31.0f;
        room_static_mesh.tint.a = 1.0f;
        return room_static_mesh;
    }
};


struct StaticMesh
{
    uint32_t id; // Object Identifier (matched in Items[])
    uint16_t mesh; // mesh (offset into MeshPointers[])
    core::BoundingBox visibility_box;
    core::BoundingBox collision_box;
    uint16_t flags; // Meaning uncertain; it is usually 2, and is 3 for objects Lara can travel through,
    // like TR2's skeletons and underwater vegetation

    bool doNotCollide() const
    {
        return (flags & 1) != 0;
    }

    core::BoundingBox getCollisionBox(const core::TRVec& pos, core::Angle angle) const;

    static std::unique_ptr<StaticMesh> read(io::SDLReader& reader)
    {
        std::unique_ptr<StaticMesh> mesh{new StaticMesh()};
        mesh->id = reader.readU32();
        mesh->mesh = reader.readU16();

        mesh->visibility_box.min.X = reader.readI16();
        mesh->visibility_box.max.X = reader.readI16();
        mesh->visibility_box.min.Y = reader.readI16();
        mesh->visibility_box.max.Y = reader.readI16();
        mesh->visibility_box.min.Z = reader.readI16();
        mesh->visibility_box.max.Z = reader.readI16();

        mesh->collision_box.min.X = reader.readI16();
        mesh->collision_box.max.X = reader.readI16();
        mesh->collision_box.min.Y = reader.readI16();
        mesh->collision_box.max.Y = reader.readI16();
        mesh->collision_box.min.Z = reader.readI16();
        mesh->collision_box.max.Z = reader.readI16();

        mesh->flags = reader.readU16();
        return mesh;
    }
};
}
