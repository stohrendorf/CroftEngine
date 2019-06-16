#pragma once

#include "color.h"
#include "core/boundingbox.h"
#include "io/util.h"
#include "texture.h"

namespace render
{
class TextureAnimator;
}

namespace loader
{
namespace file
{
struct RoomStaticMesh
{
    core::TRVec position; // world coords
    core::Angle rotation;
    int16_t darkness;              // Constant lighting; -1 means use mesh lighting
    int16_t intensity2;            // Like Intensity 1, and almost always the same value [absent from TR1 data files]
    core::StaticMeshId meshId{0u}; // which StaticMesh item to draw
    FloatColor tint;               // extracted from intensity

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
        room_static_mesh.position = readCoordinates32(reader);
        room_static_mesh.rotation = core::auToAngle(reader.readI16());
        room_static_mesh.darkness = reader.readI16();
        room_static_mesh.meshId = core::StaticMeshId::type(reader.readU16());

        // only in TR2
        room_static_mesh.intensity2 = room_static_mesh.darkness;

        room_static_mesh.tint.b = room_static_mesh.tint.g = room_static_mesh.tint.r
            = room_static_mesh.intensity2 / 16384.0f;
        room_static_mesh.tint.a = 1.0f;
        return room_static_mesh;
    }

    static RoomStaticMesh readTr2(io::SDLReader& reader)
    {
        RoomStaticMesh room_static_mesh;
        room_static_mesh.position = readCoordinates32(reader);
        room_static_mesh.rotation = core::auToAngle(reader.readI16());
        room_static_mesh.darkness = reader.readI16();
        room_static_mesh.intensity2 = reader.readI16();
        room_static_mesh.meshId = core::StaticMeshId::type(reader.readU16());
        // make consistent
        if(room_static_mesh.darkness >= 0)
            room_static_mesh.darkness = (8191 - room_static_mesh.darkness) << 2;
        if(room_static_mesh.intensity2 >= 0)
            room_static_mesh.intensity2 = (8191 - room_static_mesh.intensity2) << 2;

        room_static_mesh.tint.b = room_static_mesh.tint.g = room_static_mesh.tint.r
            = room_static_mesh.intensity2 / 16384.0f;
        room_static_mesh.tint.a = 1.0f;
        return room_static_mesh;
    }

    static RoomStaticMesh readTr3(io::SDLReader& reader)
    {
        RoomStaticMesh room_static_mesh;
        room_static_mesh.position = readCoordinates32(reader);
        room_static_mesh.rotation = core::auToAngle(reader.readI16());
        room_static_mesh.darkness = reader.readI16();
        room_static_mesh.intensity2 = reader.readI16();
        room_static_mesh.meshId = core::StaticMeshId::type(reader.readU16());

        room_static_mesh.tint.r = (room_static_mesh.darkness & 0x001F) / 62.0f;

        room_static_mesh.tint.g = ((room_static_mesh.darkness & 0x03E0) >> 5) / 62.0f;

        room_static_mesh.tint.b = ((room_static_mesh.darkness & 0x7C00) >> 10) / 62.0f;
        room_static_mesh.tint.a = 1.0f;
        return room_static_mesh;
    }

    static RoomStaticMesh readTr4(io::SDLReader& reader)
    {
        RoomStaticMesh room_static_mesh;
        room_static_mesh.position = readCoordinates32(reader);
        room_static_mesh.rotation = core::auToAngle(reader.readI16());
        room_static_mesh.darkness = reader.readI16();
        room_static_mesh.intensity2 = reader.readI16();
        room_static_mesh.meshId = core::StaticMeshId::type(reader.readU16());

        room_static_mesh.tint.r = (room_static_mesh.darkness & 0x001F) / 31.0f;

        room_static_mesh.tint.g = ((room_static_mesh.darkness & 0x03E0) >> 5) / 31.0f;

        room_static_mesh.tint.b = ((room_static_mesh.darkness & 0x7C00) >> 10) / 31.0f;
        room_static_mesh.tint.a = 1.0f;
        return room_static_mesh;
    }
};

struct StaticMesh
{
    core::StaticMeshId id{0u}; // Object Identifier (matched in Items[])
    uint16_t mesh;             // mesh (offset into MeshPointers[])
    core::BoundingBox visibility_box;
    core::BoundingBox collision_box;
    uint16_t flags; // Meaning uncertain; it is usually 2, and is 3 for objects Lara can travel through,
    // like TR2's skeletons and underwater vegetation

    bool doNotCollide() const
    {
        return (flags & 1u) != 0;
    }

    bool isVisible() const
    {
        return (flags & 2u) != 0;
    }

    core::BoundingBox getCollisionBox(const core::TRVec& pos, const core::Angle& angle) const;

    static std::unique_ptr<StaticMesh> read(io::SDLReader& reader)
    {
        std::unique_ptr<StaticMesh> mesh = std::make_unique<StaticMesh>();
        mesh->id = reader.readU32();
        mesh->mesh = reader.readU16();

        mesh->visibility_box.min.X = core::Length{static_cast<core::Length::type>(reader.readI16())};
        mesh->visibility_box.max.X = core::Length{static_cast<core::Length::type>(reader.readI16())};
        mesh->visibility_box.min.Y = core::Length{static_cast<core::Length::type>(reader.readI16())};
        mesh->visibility_box.max.Y = core::Length{static_cast<core::Length::type>(reader.readI16())};
        mesh->visibility_box.min.Z = core::Length{static_cast<core::Length::type>(reader.readI16())};
        mesh->visibility_box.max.Z = core::Length{static_cast<core::Length::type>(reader.readI16())};

        mesh->collision_box.min.X = core::Length{static_cast<core::Length::type>(reader.readI16())};
        mesh->collision_box.max.X = core::Length{static_cast<core::Length::type>(reader.readI16())};
        mesh->collision_box.min.Y = core::Length{static_cast<core::Length::type>(reader.readI16())};
        mesh->collision_box.max.Y = core::Length{static_cast<core::Length::type>(reader.readI16())};
        mesh->collision_box.min.Z = core::Length{static_cast<core::Length::type>(reader.readI16())};
        mesh->collision_box.max.Z = core::Length{static_cast<core::Length::type>(reader.readI16())};

        mesh->flags = reader.readU16();
        return mesh;
    }
};
} // namespace file
} // namespace loader
