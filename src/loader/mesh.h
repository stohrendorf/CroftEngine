#pragma once

#include "core/coordinates.h"
#include "primitives.h"
#include "texture.h"
#include "util.h"

#include <vector>


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
}
