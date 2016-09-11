#pragma once

#include "color.h"
#include "primitives.h"
#include "texture.h"
#include "util.h"

#include <map>


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
    struct Mesh
    {
        core::TRCoordinates center; // This is usually close to the mesh's centroid, and appears to be the center of a sphere used for collision testing.
        int32_t collision_size; // This appears to be the radius of that aforementioned collisional sphere.
        std::vector<core::TRCoordinates> vertices; //[NumVertices]; // list of vertices (relative coordinates)
        std::vector<core::TRCoordinates> normals; //[NumNormals]; // list of normals (if NumNormals is positive)
        std::vector<int16_t> lights; //[-NumNormals]; // list of light values (if NumNormals is negative)
        std::vector<QuadFace> textured_rectangles; //[NumTexturedRectangles]; // list of textured rectangles
        std::vector<Triangle> textured_triangles; //[NumTexturedTriangles]; // list of textured triangles
        // the rest is not present in TR4
        std::vector<QuadFace> colored_rectangles; //[NumColouredRectangles]; // list of coloured rectangles
        std::vector<Triangle> colored_triangles; //[NumColouredTriangles]; // list of coloured triangles

        /** \brief reads mesh definition.
        *
        * The read num_normals value is positive when normals are available and negative when light
        * values are available. The values get set appropiatly.
        */
        static std::unique_ptr<Mesh> readTr1(io::SDLReader& reader)
        {
            std::unique_ptr<Mesh> mesh{new Mesh()};
            mesh->center = readCoordinates16(reader);
            mesh->collision_size = reader.readI32();

            reader.readVector(mesh->vertices, reader.readI16(), &io::readCoordinates16);

            auto num_normals = reader.readI16();
            if( num_normals >= 0 )
            {
                reader.readVector(mesh->normals, num_normals, &io::readCoordinates16);
            }
            else
            {
                reader.readVector(mesh->lights, -num_normals);
            }

            reader.readVector(mesh->textured_rectangles, reader.readU16(), &QuadFace::readTr1);
            reader.readVector(mesh->textured_triangles, reader.readU16(), &Triangle::readTr1);
            reader.readVector(mesh->colored_rectangles, reader.readU16(), &QuadFace::readTr1);
            reader.readVector(mesh->colored_triangles, reader.readU16(), &Triangle::readTr1);

            return mesh;
        }


        static std::unique_ptr<Mesh> readTr4(io::SDLReader& reader)
        {
            std::unique_ptr<Mesh> mesh{new Mesh()};
            mesh->center = readCoordinates16(reader);
            mesh->collision_size = reader.readI32();

            reader.readVector(mesh->vertices, reader.readU16(), &io::readCoordinates16);

            auto num_normals = reader.readI16();
            if( num_normals >= 0 )
            {
                reader.readVector(mesh->normals, num_normals, &io::readCoordinates16);
            }
            else
            {
                reader.readVector(mesh->lights, -num_normals);
            }

            reader.readVector(mesh->textured_rectangles, reader.readU16(), &QuadFace::readTr4);
            reader.readVector(mesh->textured_triangles, reader.readU16(), &Triangle::readTr4);

            return mesh;
        }


        std::shared_ptr<gameplay::Model> createModel(const std::vector<TextureLayoutProxy>& textureProxies, const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& materials, const std::vector<std::shared_ptr<gameplay::Material>>& colorMaterials, render::TextureAnimator& animator) const;
    };


    struct RoomStaticMesh
    {
        core::TRCoordinates position; // world coords
        int16_t rotation; // high two bits (0xC000) indicate steps of
        // 90 degrees (e.g. (Rotation >> 14) * 90)
        int16_t intensity1; // Constant lighting; -1 means use mesh lighting
        int16_t intensity2; // Like Intensity 1, and almost always the same value [absent from TR1 data files]
        uint16_t meshId; // which StaticMesh item to draw
        FloatColor tint; // extracted from intensity

        /** \brief reads a room staticmesh definition.
        *
        * rotation gets converted to float and scaled appropiatly.
        * intensity1 gets converted, so it matches the 0-32768 range introduced in TR3.
        * intensity2 is introduced in TR2 and is set to intensity1 for TR1.
        */
        static RoomStaticMesh readTr1(io::SDLReader& reader)
        {
            RoomStaticMesh room_static_mesh;
            room_static_mesh.position = readCoordinates32(reader);
            room_static_mesh.rotation = reader.readI16();
            room_static_mesh.intensity1 = reader.readI16();
            room_static_mesh.meshId = reader.readU16();
            // make consistent
            if( room_static_mesh.intensity1 >= 0 )
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
            room_static_mesh.position = readCoordinates32(reader);
            room_static_mesh.rotation = reader.readI16();
            room_static_mesh.intensity1 = reader.readI16();
            room_static_mesh.intensity2 = reader.readI16();
            room_static_mesh.meshId = reader.readU16();
            // make consistent
            if( room_static_mesh.intensity1 >= 0 )
                room_static_mesh.intensity1 = (8191 - room_static_mesh.intensity1) << 2;
            if( room_static_mesh.intensity2 >= 0 )
                room_static_mesh.intensity2 = (8191 - room_static_mesh.intensity2) << 2;

            room_static_mesh.tint.b = room_static_mesh.tint.g = room_static_mesh.tint.r = room_static_mesh.intensity2 / 16384.0f;
            room_static_mesh.tint.a = 1.0f;
            return room_static_mesh;
        }


        static RoomStaticMesh readTr3(io::SDLReader& reader)
        {
            RoomStaticMesh room_static_mesh;
            room_static_mesh.position = readCoordinates32(reader);
            room_static_mesh.rotation = reader.readI16();
            room_static_mesh.intensity1 = reader.readI16();
            room_static_mesh.intensity2 = reader.readI16();
            room_static_mesh.meshId = reader.readU16();

            room_static_mesh.tint.r = (room_static_mesh.intensity1 & 0x001F) / 62.0f;

            room_static_mesh.tint.g = ((room_static_mesh.intensity1 & 0x03E0) >> 5) / 62.0f;

            room_static_mesh.tint.b = ((room_static_mesh.intensity1 & 0x7C00) >> 10) / 62.0f;
            room_static_mesh.tint.a = 1.0f;
            return room_static_mesh;
        }


        static RoomStaticMesh readTr4(io::SDLReader& reader)
        {
            RoomStaticMesh room_static_mesh;
            room_static_mesh.position = readCoordinates32(reader);
            room_static_mesh.rotation = reader.readI16();
            room_static_mesh.intensity1 = reader.readI16();
            room_static_mesh.intensity2 = reader.readI16();
            room_static_mesh.meshId = reader.readU16();

            room_static_mesh.tint.r = (room_static_mesh.intensity1 & 0x001F) / 31.0f;

            room_static_mesh.tint.g = ((room_static_mesh.intensity1 & 0x03E0) >> 5) / 31.0f;

            room_static_mesh.tint.b = ((room_static_mesh.intensity1 & 0x7C00) >> 10) / 31.0f;
            room_static_mesh.tint.a = 1.0f;
            return room_static_mesh;
        }
    };


    struct StaticMesh
    {
        uint32_t id; // Object Identifier (matched in Items[])
        uint16_t mesh; // mesh (offset into MeshPointers[])
        gameplay::BoundingBox visibility_box;
        gameplay::BoundingBox collision_box;
        uint16_t flags; // Meaning uncertain; it is usually 2, and is 3 for objects Lara can travel through,
        // like TR2's skeletons and underwater vegetation

        bool doNotCollide() const
        {
            return (flags & 1) != 0;
        }


        gameplay::BoundingBox getCollisionBox(const core::TRCoordinates& pos, core::Angle angle) const;


        static std::unique_ptr<StaticMesh> read(io::SDLReader& reader)
        {
            std::unique_ptr<StaticMesh> mesh{new StaticMesh()};
            mesh->id = reader.readU32();
            mesh->mesh = reader.readU16();

            mesh->visibility_box.min.x = reader.readI16();
            mesh->visibility_box.max.x = reader.readI16();
            mesh->visibility_box.min.y = reader.readI16();
            mesh->visibility_box.max.y = reader.readI16();
            mesh->visibility_box.min.z = reader.readI16();
            mesh->visibility_box.max.z = reader.readI16();

            mesh->collision_box.min.x = reader.readI16();
            mesh->collision_box.max.x = reader.readI16();
            mesh->collision_box.min.y = reader.readI16();
            mesh->collision_box.max.y = reader.readI16();
            mesh->collision_box.min.z = reader.readI16();
            mesh->collision_box.max.z = reader.readI16();

            mesh->flags = reader.readU16();
            return mesh;
        }
    };
}
