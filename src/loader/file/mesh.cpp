#include "mesh.h"

#include "color.h"
#include "io/sdlreader.h"
#include "io/util.h"
#include "render/gl/vertexarray.h"
#include "render/scene/mesh.h"
#include "render/scene/model.h"
#include "render/scene/uniformparameter.h"
#include "util.h"

#include <utility>

namespace loader::file
{
namespace
{
class ModelBuilder final
{
  struct RenderVertex
  {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color{1.0f};
    glm::vec2 uv;

    static const render::gl::StructureLayout<RenderVertex>& getFormat()
    {
      static const render::gl::StructureLayout<RenderVertex> attribs{
        {VERTEX_ATTRIBUTE_POSITION_NAME, &RenderVertex::position},
        {VERTEX_ATTRIBUTE_NORMAL_NAME, &RenderVertex::normal},
        {VERTEX_ATTRIBUTE_COLOR_NAME, &RenderVertex::color},
        {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &RenderVertex::uv}};

      return attribs;
    }
  };

  const bool m_hasNormals;
  std::vector<RenderVertex> m_vertices;
  const std::vector<TextureTile>& m_textureTiles;
  const std::map<TextureKey, gsl::not_null<std::shared_ptr<render::scene::Material>>>& m_materialsFull;
  const gsl::not_null<std::shared_ptr<render::scene::Material>> m_colorMaterialFull;
  const gsl::not_null<std::shared_ptr<render::scene::Material>> m_materialDepthOnly;
  const Palette& m_palette;
  std::map<TextureKey, size_t> m_texBuffers;
  std::shared_ptr<render::gl::StructuredArrayBuffer<RenderVertex>> m_vb;
  const std::string m_label;

  struct MeshPart
  {
    using IndexBuffer = std::vector<uint16_t>;

    IndexBuffer indices;
    std::shared_ptr<render::scene::Material> materialFull;
    std::shared_ptr<render::scene::Material> materialDepthOnly;
    std::optional<glm::vec3> color;
  };

  std::vector<MeshPart> m_parts;

  void append(const RenderVertex& v);

  size_t getPartForColor(const core::TextureTileId tileId)
  {
    TextureKey tk;
    tk.blendingMode = BlendingMode::Solid;
    tk.flags = 0;
    tk.tileAndFlag = 0;
    tk.colorId = tileId.get() & 0xff;
    const auto color = gsl::at(m_palette.colors, tk.colorId.get()).toGLColor3();

    if(m_texBuffers.find(tk) == m_texBuffers.end())
    {
      m_texBuffers[tk] = m_parts.size();
      m_parts.emplace_back();
      m_parts.back().materialFull = m_colorMaterialFull;
      m_parts.back().materialDepthOnly = m_materialDepthOnly;
      m_parts.back().color = color;
    }

    return m_texBuffers[tk];
  }

  size_t getPartForTexture(const TextureTile& tile)
  {
    if(m_texBuffers.find(tile.textureKey) == m_texBuffers.end())
    {
      m_texBuffers[tile.textureKey] = m_parts.size();
      m_parts.emplace_back();
      m_parts.back().materialFull = m_materialsFull.at(tile.textureKey);
      m_parts.back().materialDepthOnly = m_materialDepthOnly;
    }
    return m_texBuffers[tile.textureKey];
  }

public:
  explicit ModelBuilder(
    bool withNormals,
    const std::vector<TextureTile>& textureTiles,
    const std::map<TextureKey, gsl::not_null<std::shared_ptr<render::scene::Material>>>& materialsFull,
    gsl::not_null<std::shared_ptr<render::scene::Material>> colorMaterialFull,
    gsl::not_null<std::shared_ptr<render::scene::Material>> materialDepthOnly,
    const Palette& palette,
    std::string label = {})
      : m_hasNormals{withNormals}
      , m_textureTiles{textureTiles}
      , m_materialsFull{materialsFull}
      , m_colorMaterialFull{std::move(colorMaterialFull)}
      , m_materialDepthOnly{std::move(materialDepthOnly)}
      , m_palette{palette}
      , m_vb{std::make_shared<render::gl::StructuredArrayBuffer<RenderVertex>>(RenderVertex::getFormat(), label)}
      , m_label{std::move(label)}
  {
  }

  void append(const Mesh& mesh);

  gsl::not_null<std::shared_ptr<render::scene::Model>> finalize();
};

void ModelBuilder::append(const RenderVertex& v)
{
  m_vertices.emplace_back(v);
}

void ModelBuilder::append(const Mesh& mesh)
{
  if(mesh.normals.empty() && m_hasNormals)
    BOOST_THROW_EXCEPTION(std::runtime_error("Trying to append a mesh with normals to a buffer without normals"));
  else if(!mesh.normals.empty() && !m_hasNormals)
    BOOST_THROW_EXCEPTION(std::runtime_error("Trying to append a mesh without normals to a buffer with normals"));

  for(const QuadFace& quad : mesh.textured_rectangles)
  {
    const TextureTile& tile = m_textureTiles.at(quad.tileId.get());
    const auto partId = getPartForTexture(tile);

    glm::vec3 defaultNormal{0.0f};
    if(m_hasNormals)
    {
      for(auto v : quad.vertices)
      {
        const auto n = v.from(mesh.normals).toRenderSystem();
        if(n != glm::vec3{0.0f})
        {
          defaultNormal = n;
          break;
        }
      }
    }

    const auto firstVertex = m_vertices.size();
    for(int i = 0; i < 4; ++i)
    {
      RenderVertex iv{};

      if(!m_hasNormals)
      {
        iv.color = glm::vec3(1 - quad.vertices[i].from(mesh.vertexDarknesses) / 8191.0f);
        if(i <= 2)
        {
          static const int indices[3] = {0, 1, 2};
          iv.normal = generateNormal(quad.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                     quad.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                     quad.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
        }
        else
        {
          static const int indices[3] = {0, 2, 3};
          iv.normal = generateNormal(quad.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                     quad.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                     quad.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
        }
      }
      else
      {
        iv.normal = quad.vertices[i].from(mesh.normals).toRenderSystem();
        if(iv.normal == glm::vec3{0.0f})
          iv.normal = defaultNormal;
      }

      iv.position = quad.vertices[i].from(mesh.vertices).toRenderSystem();
      iv.uv = tile.uvCoordinates[i].toGl();
      append(iv);
    }

    for(auto i : {0, 1, 2, 0, 2, 3})
    {
      m_parts[partId].indices.emplace_back(gsl::narrow<MeshPart::IndexBuffer::value_type>(firstVertex + i));
    }
  }
  for(const QuadFace& quad : mesh.colored_rectangles)
  {
    const TextureTile& tile = m_textureTiles.at(quad.tileId.get());
    const auto partId = getPartForColor(quad.tileId);
    const auto color = *m_parts[partId].color;

    glm::vec3 defaultNormal{0.0f};
    if(m_hasNormals)
    {
      for(auto v : quad.vertices)
      {
        const auto n = v.from(mesh.normals).toRenderSystem();
        if(n != glm::vec3{0.0f})
        {
          defaultNormal = n;
          break;
        }
      }
    }

    const auto firstVertex = m_vertices.size();
    for(int i = 0; i < 4; ++i)
    {
      RenderVertex iv{};
      iv.position = quad.vertices[i].from(mesh.vertices).toRenderSystem();
      iv.color = color;
      if(!m_hasNormals)
      {
        iv.color *= 1 - quad.vertices[i].from(mesh.vertexDarknesses) / 8191.0f;
        if(i <= 2)
        {
          static const int indices[3] = {0, 1, 2};
          iv.normal = generateNormal(quad.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                     quad.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                     quad.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
        }
        else
        {
          static const int indices[3] = {0, 2, 3};
          iv.normal = generateNormal(quad.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                     quad.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                     quad.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
        }
      }
      else
      {
        iv.normal = quad.vertices[i].from(mesh.normals).toRenderSystem();
        if(iv.normal == glm::vec3{0.0f})
          iv.normal = defaultNormal;
      }
      iv.uv = tile.uvCoordinates[i].toGl();
      append(iv);
    }
    for(auto i : {0, 1, 2, 0, 2, 3})
    {
      m_parts[partId].indices.emplace_back(gsl::narrow<MeshPart::IndexBuffer::value_type>(firstVertex + i));
    }
  }
  for(const Triangle& tri : mesh.textured_triangles)
  {
    const TextureTile& tile = m_textureTiles.at(tri.tileId.get());
    const auto partId = getPartForTexture(tile);

    glm::vec3 defaultNormal{0.0f};
    if(m_hasNormals)
    {
      for(auto v : tri.vertices)
      {
        const auto n = v.from(mesh.normals).toRenderSystem();
        if(n != glm::vec3{0.0f})
        {
          defaultNormal = n;
          break;
        }
      }
    }

    for(int i = 0; i < 3; ++i)
    {
      RenderVertex iv{};
      iv.position = tri.vertices[i].from(mesh.vertices).toRenderSystem();
      iv.uv = tile.uvCoordinates[i].toGl();
      if(!m_hasNormals)
      {
        iv.color = glm::vec3(1 - tri.vertices[i].from(mesh.vertexDarknesses) / 8191.0f);

        static const int indices[3] = {0, 1, 2};
        iv.normal = generateNormal(tri.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                   tri.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                   tri.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
      }
      else
      {
        iv.normal = tri.vertices[i].from(mesh.normals).toRenderSystem();
        if(iv.normal == glm::vec3{0.0f})
          iv.normal = defaultNormal;
      }
      m_parts[partId].indices.emplace_back(gsl::narrow<MeshPart::IndexBuffer::value_type>(m_vertices.size()));
      append(iv);
    }
  }
  for(const Triangle& tri : mesh.colored_triangles)
  {
    const TextureTile& tile = m_textureTiles.at(tri.tileId.get());
    const auto partId = getPartForColor(tri.tileId);
    const auto color = *m_parts[partId].color;

    glm::vec3 defaultNormal{0.0f};
    if(m_hasNormals)
    {
      for(auto v : tri.vertices)
      {
        const auto n = v.from(mesh.normals).toRenderSystem();
        if(n != glm::vec3{0.0f})
        {
          defaultNormal = n;
          break;
        }
      }
    }

    for(int i = 0; i < 3; ++i)
    {
      RenderVertex iv{};
      iv.position = tri.vertices[i].from(mesh.vertices).toRenderSystem();
      iv.color = color;
      if(!m_hasNormals)
      {
        iv.color *= glm::vec3(1 - tri.vertices[i].from(mesh.vertexDarknesses) / 8191.0f);

        static const int indices[3] = {0, 1, 2};
        iv.normal = generateNormal(tri.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                   tri.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                   tri.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
      }
      else
      {
        iv.normal = tri.vertices[i].from(mesh.normals).toRenderSystem();
        if(iv.normal == glm::vec3{0.0f})
          iv.normal = defaultNormal;
      }
      iv.uv = tile.uvCoordinates[i].toGl();
      m_parts[partId].indices.emplace_back(gsl::narrow<MeshPart::IndexBuffer::value_type>(m_vertices.size()));
      append(iv);
    }
  }
}

gsl::not_null<std::shared_ptr<render::scene::Model>> ModelBuilder::finalize()
{
  m_vb->setData(m_vertices, gl::BufferUsageARB::StaticDraw);

  auto model = std::make_shared<render::scene::Model>();
  for(const MeshPart& localPart : m_parts)
  {
#ifndef NDEBUG
    for(auto idx : localPart.indices)
    {
      BOOST_ASSERT(idx < m_vertices.size());
    }
#endif

    auto indexBuffer = std::make_shared<render::gl::ElementArrayBuffer<uint16_t>>();
    indexBuffer->setData(localPart.indices, gl::BufferUsageARB::DynamicDraw);

    auto va = std::make_shared<render::gl::VertexArray<uint16_t, RenderVertex>>(
      indexBuffer,
      m_vb,
      std::vector<const render::gl::Program*>{&localPart.materialFull->getShaderProgram()->getHandle(),
                                              localPart.materialDepthOnly == nullptr
                                                ? nullptr
                                                : &localPart.materialDepthOnly->getShaderProgram()->getHandle()},
      m_label);
    auto mesh = std::make_shared<render::scene::MeshImpl<uint16_t, RenderVertex>>(va, gl::PrimitiveType::Triangles);
    mesh->getMaterial()
      .set(render::scene::RenderMode::Full, localPart.materialFull)
      .set(render::scene::RenderMode::DepthOnly, localPart.materialDepthOnly);

    model->addMesh(mesh);
  }

  return model;
}
} // namespace

std::shared_ptr<render::scene::Model>
  Mesh::createModel(const std::vector<TextureTile>& textureTiles,
                    const std::map<TextureKey, gsl::not_null<std::shared_ptr<render::scene::Material>>>& materialsFull,
                    gsl::not_null<std::shared_ptr<render::scene::Material>> colorMaterialFull,
                    gsl::not_null<std::shared_ptr<render::scene::Material>> materialDepthOnly,
                    const Palette& palette,
                    const std::string& label) const
{
  ModelBuilder mb{!normals.empty(),
                  textureTiles,
                  materialsFull,
                  std::move(colorMaterialFull),
                  std::move(materialDepthOnly),
                  palette,
                  label};

  mb.append(*this);

  return mb.finalize();
}

std::unique_ptr<Mesh> Mesh::readTr1(io::SDLReader& reader)
{
  std::unique_ptr<Mesh> mesh{std::make_unique<Mesh>()};
  mesh->center = readCoordinates16(reader);
  mesh->collision_size = core::Length{core::Length::type{reader.readI16()}};
  reader.skip(2); // some unknown flags

  reader.readVector(mesh->vertices, reader.readU16(), &io::readCoordinates16);

  const auto num_normals = reader.readI16();
  if(num_normals >= 0)
  {
    Expects(static_cast<size_t>(num_normals) == mesh->vertices.size());
    reader.readVector(mesh->normals, num_normals, &io::readCoordinates16);
  }
  else
  {
    Expects(static_cast<size_t>(-num_normals) == mesh->vertices.size());
    reader.readVector(mesh->vertexDarknesses, -num_normals);
  }

  reader.readVector(mesh->textured_rectangles, reader.readU16(), &QuadFace::readTr1);
  reader.readVector(mesh->textured_triangles, reader.readU16(), &Triangle::readTr1);
  reader.readVector(mesh->colored_rectangles, reader.readU16(), &QuadFace::readTr1);
  reader.readVector(mesh->colored_triangles, reader.readU16(), &Triangle::readTr1);

  return mesh;
}

std::unique_ptr<Mesh> Mesh::readTr4(io::SDLReader& reader)
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
} // namespace loader::file
