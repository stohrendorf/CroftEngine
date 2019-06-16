#pragma once

#include "color.h"
#include "core/vec.h"
#include "io/util.h"
#include "primitives.h"
#include "render/scene/mesh.h"
#include "texture.h"

#include <vector>

namespace render
{
class TextureAnimator;

namespace scene
{
class Model;
}
} // namespace render

namespace loader
{
namespace file
{
struct Mesh
{
    core::TRVec
        center; // This is usually close to the mesh's centroid, and appears to be the center of a sphere used for collision testing.
    core::Length collision_size = 0_len;   // This appears to be the radius of that aforementioned collisional sphere.
    std::vector<core::TRVec> vertices;     //[NumVertices]; // list of vertices (relative coordinates)
    std::vector<core::TRVec> normals;      //[NumNormals]; // list of normals (if NumNormals is positive)
    std::vector<int16_t> vertexDarknesses; //[-NumNormals]; // list of light values (if NumNormals is negative), 0..8191
    std::vector<QuadFace> textured_rectangles; //[NumTexturedRectangles]; // list of textured rectangles
    std::vector<Triangle> textured_triangles;  //[NumTexturedTriangles]; // list of textured triangles
    // the rest is not present in TR4
    std::vector<QuadFace> colored_rectangles; //[NumColoredRectangles]; // list of colored rectangles
    std::vector<Triangle> colored_triangles;  //[NumColoredTriangles]; // list of colored triangles

    /** \brief reads mesh definition.
    *
    * The read num_normals value is positive when normals are available and negative when light
    * values are available. The values get set appropriately.
    */
    static std::unique_ptr<Mesh> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<Mesh> mesh{std::make_unique<Mesh>()};
        mesh->center = readCoordinates16(reader);
        mesh->collision_size = core::Length{core::Length::type{reader.readI16()}};
        reader.skip(2); // some unknown flags

        reader.readVector(mesh->vertices, reader.readU16(), &io::readCoordinates16);

        const auto num_normals = reader.readI16();
        if(num_normals >= 0)
        {
            Expects(num_normals == mesh->vertices.size());
            reader.readVector(mesh->normals, num_normals, &io::readCoordinates16);
        }
        else
        {
            Expects(-num_normals == mesh->vertices.size());
            reader.readVector(mesh->vertexDarknesses, -num_normals);
        }

        reader.readVector(mesh->textured_rectangles, reader.readU16(), &QuadFace::readTr1);
        reader.readVector(mesh->textured_triangles, reader.readU16(), &Triangle::readTr1);
        reader.readVector(mesh->colored_rectangles, reader.readU16(), &QuadFace::readTr1);
        reader.readVector(mesh->colored_triangles, reader.readU16(), &Triangle::readTr1);

        return mesh;
    }

    static std::unique_ptr<Mesh> readTr4(io::SDLReader& reader)
    {
        std::unique_ptr<Mesh> mesh{std::make_unique<Mesh>()};
        mesh->center = readCoordinates16(reader);
        mesh->collision_size = core::Length{reader.readI32()};

        reader.readVector(mesh->vertices, reader.readU16(), &io::readCoordinates16);

        const auto num_normals = reader.readI16();
        if(num_normals >= 0)
        {
            reader.readVector(mesh->normals, num_normals, &io::readCoordinates16);
        }
        else
        {
            reader.readVector(mesh->vertexDarknesses, -num_normals);
        }

        reader.readVector(mesh->textured_rectangles, reader.readU16(), &QuadFace::readTr4);
        reader.readVector(mesh->textured_triangles, reader.readU16(), &Triangle::readTr4);

        return mesh;
    }

    class ModelBuilder
    {
        struct RenderVertex;

        const bool m_hasNormals;
        std::vector<float> m_vbuf;
        const std::vector<TextureLayoutProxy>& m_textureProxies;
        const std::map<TextureKey, gsl::not_null<std::shared_ptr<render::scene::Material>>>& m_materials;
        const gsl::not_null<std::shared_ptr<render::scene::Material>> m_colorMaterial;
        const Palette& m_palette;
        std::map<TextureKey, size_t> m_texBuffers;
        size_t m_vertexCount = 0;
        std::shared_ptr<render::gl::StructuredVertexBuffer> m_vb;
        const std::string m_label;

        struct MeshPart
        {
            using IndexBuffer = std::vector<uint16_t>;

            IndexBuffer indices;
            std::shared_ptr<render::scene::Material> material;
            boost::optional<glm::vec3> color;
        };

        std::vector<MeshPart> m_parts;

        void append(const RenderVertex& v);

        size_t getPartForColor(const core::TextureProxyId proxyId)
        {
            TextureKey tk;
            tk.blendingMode = BlendingMode::Solid;
            tk.flags = 0;
            tk.tileAndFlag = 0;
            tk.colorId = proxyId.get() & 0xff;
            const auto color = gsl::at(m_palette.colors, tk.colorId.get()).toGLColor3();

            if(m_texBuffers.find(tk) == m_texBuffers.end())
            {
                m_texBuffers[tk] = m_parts.size();
                m_parts.emplace_back();
                m_parts.back().material = m_colorMaterial;
                m_parts.back().color = color;
            }

            return m_texBuffers[tk];
        }

        size_t getPartForTexture(const TextureLayoutProxy& proxy)
        {
            if(m_texBuffers.find(proxy.textureKey) == m_texBuffers.end())
            {
                m_texBuffers[proxy.textureKey] = m_parts.size();
                m_parts.emplace_back();
                m_parts.back().material = m_materials.at(proxy.textureKey);
            }
            return m_texBuffers[proxy.textureKey];
        }

    public:
        explicit ModelBuilder(
            bool withNormals,
            bool dynamic,
            const std::vector<TextureLayoutProxy>& textureProxies,
            const std::map<TextureKey, gsl::not_null<std::shared_ptr<render::scene::Material>>>& materials,
            gsl::not_null<std::shared_ptr<render::scene::Material>> colorMaterial,
            const Palette& palette,
            const std::string& label = {});

        ~ModelBuilder();

        void append(const Mesh& mesh);

        gsl::not_null<std::shared_ptr<render::scene::Model>> finalize();
    };

    std::shared_ptr<render::scene::Model>
        createModel(const std::vector<TextureLayoutProxy>& textureProxies,
                    const std::map<TextureKey, gsl::not_null<std::shared_ptr<render::scene::Material>>>& materials,
                    const gsl::not_null<std::shared_ptr<render::scene::Material>>& colorMaterial,
                    const Palette& palette,
                    const std::string& label = {}) const;
};
} // namespace file
} // namespace loader
