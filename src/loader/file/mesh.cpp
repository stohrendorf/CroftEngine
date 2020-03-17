#include "mesh.h"

#include "color.h"
#include "datatypes.h"
#include "io/sdlreader.h"
#include "io/util.h"
#include "render/scene/mesh.h"
#include "util.h"

#include <gl/vertexarray.h>
#include <utility>

namespace loader::file
{
namespace
{
class RenderMeshBuilder final
{
  using IndexType = uint16_t;

  struct RenderVertex
  {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color{1.0f};
    glm::vec2 uv;
    glm::int32_t textureIndex{-1};

    static const gl::VertexFormat<RenderVertex>& getFormat()
    {
      static const gl::VertexFormat<RenderVertex> format{{VERTEX_ATTRIBUTE_POSITION_NAME, &RenderVertex::position},
                                                         {VERTEX_ATTRIBUTE_NORMAL_NAME, &RenderVertex::normal},
                                                         {VERTEX_ATTRIBUTE_COLOR_NAME, &RenderVertex::color},
                                                         {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &RenderVertex::uv},
                                                         {VERTEX_ATTRIBUTE_TEXINDEX_NAME, &RenderVertex::textureIndex}};

      return format;
    }
  };

  const bool m_hasNormals;
  std::vector<RenderVertex> m_vertices;
  const std::vector<TextureTile>& m_textureTiles;
  const gsl::not_null<std::shared_ptr<render::scene::Material>> m_materialFull;
  const gsl::not_null<std::shared_ptr<render::scene::Material>> m_materialDepthOnly;
  const gsl::not_null<std::shared_ptr<render::scene::Material>> m_materialCSMDepthOnly;
  const Palette& m_palette;
  std::shared_ptr<gl::VertexBuffer<RenderVertex>> m_vb;
  const std::string m_label;
  std::vector<IndexType> m_indices{};

  void append(const RenderVertex& v);

public:
  explicit RenderMeshBuilder(bool withNormals,
                             const std::vector<TextureTile>& textureTiles,
                             gsl::not_null<std::shared_ptr<render::scene::Material>> materialFull,
                             gsl::not_null<std::shared_ptr<render::scene::Material>> materialDepthOnly,
                             gsl::not_null<std::shared_ptr<render::scene::Material>> materialCSMDepthOnly,
                             const Palette& palette,
                             std::string label = {})
      : m_hasNormals{withNormals}
      , m_textureTiles{textureTiles}
      , m_materialFull{std::move(materialFull)}
      , m_materialDepthOnly{std::move(materialDepthOnly)}
      , m_materialCSMDepthOnly{std::move(materialCSMDepthOnly)}
      , m_palette{palette}
      , m_vb{std::make_shared<gl::VertexBuffer<RenderVertex>>(RenderVertex::getFormat(), label)}
      , m_label{std::move(label)}
  {
  }

  void append(const Mesh& mesh);

  gsl::not_null<std::shared_ptr<render::scene::Mesh>> finalize();
};

void RenderMeshBuilder::append(const RenderVertex& v)
{
  m_vertices.emplace_back(v);
}

void RenderMeshBuilder::append(const Mesh& mesh)
{
  if(mesh.normals.empty() && m_hasNormals)
    BOOST_THROW_EXCEPTION(std::runtime_error("Trying to append a mesh with normals to a buffer without normals"));
  else if(!mesh.normals.empty() && !m_hasNormals)
    BOOST_THROW_EXCEPTION(std::runtime_error("Trying to append a mesh without normals to a buffer with normals"));

  for(const QuadFace& quad : mesh.textured_rectangles)
  {
    const TextureTile& tile = m_textureTiles.at(quad.tileId.get());

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
      iv.textureIndex = tile.textureKey.tileAndFlag & loader::file::TextureIndexMask;

      if(!m_hasNormals)
      {
        iv.color = glm::vec3(toBrightness(quad.vertices[i].from(mesh.vertexShades)).get());
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
      m_indices.emplace_back(gsl::narrow<IndexType>(firstVertex + i));
    }
  }
  for(const QuadFace& quad : mesh.colored_rectangles)
  {
    const TextureTile& tile = m_textureTiles.at(quad.tileId.get());
    const auto color = gsl::at(m_palette.colors, quad.tileId.get() & 0xff).toGLColor3();

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
      iv.textureIndex = -1;
      iv.color = color;
      if(!m_hasNormals)
      {
        iv.color *= toBrightness(quad.vertices[i].from(mesh.vertexShades)).get();
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
      m_indices.emplace_back(gsl::narrow<IndexType>(firstVertex + i));
    }
  }

  for(const Triangle& tri : mesh.textured_triangles)
  {
    const TextureTile& tile = m_textureTiles.at(tri.tileId.get());

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
      iv.textureIndex = tile.textureKey.tileAndFlag & loader::file::TextureIndexMask;
      iv.uv = tile.uvCoordinates[i].toGl();
      if(!m_hasNormals)
      {
        iv.color = glm::vec3(toBrightness(tri.vertices[i].from(mesh.vertexShades)).get());

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
      m_indices.emplace_back(gsl::narrow<IndexType>(m_vertices.size()));
      append(iv);
    }
  }

  for(const Triangle& tri : mesh.colored_triangles)
  {
    const TextureTile& tile = m_textureTiles.at(tri.tileId.get());
    const auto color = gsl::at(m_palette.colors, tri.tileId.get() & 0xff).toGLColor3();

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
      iv.textureIndex = -1;
      iv.color = color;
      if(!m_hasNormals)
      {
        iv.color *= glm::vec3(toBrightness(tri.vertices[i].from(mesh.vertexShades)).get());

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
      m_indices.emplace_back(gsl::narrow<IndexType>(m_vertices.size()));
      append(iv);
    }
  }
}

gsl::not_null<std::shared_ptr<render::scene::Mesh>> RenderMeshBuilder::finalize()
{
  m_vb->setData(m_vertices, gl::api::BufferUsageARB::StaticDraw);

#ifndef NDEBUG
  for(auto idx : m_indices)
  {
    BOOST_ASSERT(idx < m_vertices.size());
  }
#endif
  auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<uint16_t>>();
  indexBuffer->setData(m_indices, gl::api::BufferUsageARB::DynamicDraw);

  auto va = std::make_shared<gl::VertexArray<uint16_t, RenderVertex>>(
    indexBuffer,
    m_vb,
    std::vector<const gl::Program*>{
      &m_materialFull->getShaderProgram()->getHandle(),
      m_materialCSMDepthOnly == nullptr ? nullptr : &m_materialCSMDepthOnly->getShaderProgram()->getHandle(),
      m_materialDepthOnly == nullptr ? nullptr : &m_materialDepthOnly->getShaderProgram()->getHandle()},
    m_label);
  auto mesh = std::make_shared<render::scene::MeshImpl<uint16_t, RenderVertex>>(va, gl::api::PrimitiveType::Triangles);
  mesh->getMaterial()
    .set(render::scene::RenderMode::Full, m_materialFull)
    .set(render::scene::RenderMode::DepthOnly, m_materialDepthOnly)
    .set(render::scene::RenderMode::CSMDepthOnly, m_materialCSMDepthOnly);

  return mesh;
}
} // namespace

std::shared_ptr<render::scene::Mesh>
  Mesh::toRenderMesh(const std::vector<TextureTile>& textureTiles,
                     const gsl::not_null<std::shared_ptr<render::scene::Material>>& materialFull,
                     gsl::not_null<std::shared_ptr<render::scene::Material>> materialDepthOnly,
                     gsl::not_null<std::shared_ptr<render::scene::Material>> materialCSMDepthOnly,
                     const Palette& palette,
                     const std::string& label) const
{
  RenderMeshBuilder mb{!normals.empty(),
                       textureTiles,
                       materialFull,
                       std::move(materialDepthOnly),
                       std::move(materialCSMDepthOnly),
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
    reader.readVector(mesh->vertexShades, -num_normals);
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
    reader.readVector(mesh->vertexShades, -num_normals);
  }

  reader.readVector(mesh->textured_rectangles, reader.readU16(), &QuadFace::readTr4);
  reader.readVector(mesh->textured_triangles, reader.readU16(), &Triangle::readTr4);

  return mesh;
}
} // namespace loader::file
