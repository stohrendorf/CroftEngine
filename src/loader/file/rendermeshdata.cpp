#include "rendermeshdata.h"

namespace loader::file
{
gsl::not_null<std::shared_ptr<render::scene::Mesh>>
  RenderMeshData::toMesh(const gsl::not_null<std::shared_ptr<render::scene::Material>>& materialFull,
                         const gsl::not_null<std::shared_ptr<render::scene::Material>>& materialDepthOnly,
                         const gsl::not_null<std::shared_ptr<render::scene::Material>>& materialCSMDepthOnly,
                         const std::string& label)
{
  auto vb = std::make_shared<gl::VertexBuffer<RenderVertex>>(RenderVertex::getFormat(), label);
  vb->setData(m_vertices, gl::api::BufferUsageARB::StaticDraw);

#ifndef NDEBUG
  for(auto idx : m_indices)
  {
    BOOST_ASSERT(idx < m_vertices.size());
  }
#endif
  auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<IndexType>>();
  indexBuffer->setData(m_indices, gl::api::BufferUsageARB::DynamicDraw);

  auto va = std::make_shared<gl::VertexArray<uint16_t, RenderVertex>>(
    indexBuffer,
    vb,
    std::vector<const gl::Program*>{
      &materialFull->getShaderProgram()->getHandle(),
      materialCSMDepthOnly == nullptr ? nullptr : &materialCSMDepthOnly->getShaderProgram()->getHandle(),
      materialDepthOnly == nullptr ? nullptr : &materialDepthOnly->getShaderProgram()->getHandle()},
    label);
  auto mesh = std::make_shared<render::scene::MeshImpl<uint16_t, RenderVertex>>(va, gl::api::PrimitiveType::Triangles);
  mesh->getMaterial()
    .set(render::scene::RenderMode::Full, materialFull)
    .set(render::scene::RenderMode::DepthOnly, materialDepthOnly)
    .set(render::scene::RenderMode::CSMDepthOnly, materialCSMDepthOnly);

  return mesh;
}

RenderMeshData::RenderMeshData(const Mesh& mesh, const std::vector<TextureTile>& textureTiles, const Palette& palette)
{
  for(const QuadFace& quad : mesh.textured_rectangles)
  {
    const TextureTile& tile = textureTiles.at(quad.tileId.get());

    glm::vec3 defaultNormal{0.0f};
    if(!mesh.normals.empty())
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

      if(mesh.normals.empty())
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
      m_vertices.emplace_back(iv);
    }

    for(auto i : {0, 1, 2, 0, 2, 3})
    {
      m_indices.emplace_back(gsl::narrow<IndexType>(firstVertex + i));
    }
  }
  for(const QuadFace& quad : mesh.colored_rectangles)
  {
    const TextureTile& tile = textureTiles.at(quad.tileId.get());
    const auto color = gsl::at(palette.colors, quad.tileId.get() & 0xffu).toGLColor3();

    glm::vec3 defaultNormal{0.0f};
    if(!mesh.normals.empty())
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
      if(mesh.normals.empty())
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
      m_vertices.emplace_back(iv);
    }
    for(auto i : {0, 1, 2, 0, 2, 3})
    {
      m_indices.emplace_back(gsl::narrow<IndexType>(firstVertex + i));
    }
  }

  for(const Triangle& tri : mesh.textured_triangles)
  {
    const TextureTile& tile = textureTiles.at(tri.tileId.get());

    glm::vec3 defaultNormal{0.0f};
    if(!mesh.normals.empty())
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
      if(mesh.normals.empty())
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
      m_vertices.emplace_back(iv);
    }
  }

  for(const Triangle& tri : mesh.colored_triangles)
  {
    const TextureTile& tile = textureTiles.at(tri.tileId.get());
    const auto color = gsl::at(palette.colors, tri.tileId.get() & 0xffu).toGLColor3();

    glm::vec3 defaultNormal{0.0f};
    if(!mesh.normals.empty())
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
      if(mesh.normals.empty())
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
      m_vertices.emplace_back(iv);
    }
  }
}
} // namespace loader::file
