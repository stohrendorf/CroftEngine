#include "rendermeshdata.h"

#include "core/containeroffset.h"
#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/world/atlastile.h"
#include "engine/world/util.h"
#include "loader/file/datatypes.h"
#include "loader/file/mesh.h"
#include "loader/file/primitives.h"
#include "loader/file/texture.h"
#include "render/scene/material.h"
#include "render/scene/materialgroup.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/rendermode.h"
#include "render/scene/shaderprogram.h"

#include <boost/assert.hpp>
#include <cstddef>
#include <gl/buffer.h>
#include <gl/pixel.h>
#include <gl/renderstate.h>
#include <gl/vertexarray.h>
#include <gl/vertexbuffer.h>
#include <initializer_list>

namespace gl
{
class Program;
}

namespace engine::world
{
RenderMeshData::RenderMeshData(const loader::file::Mesh& mesh,
                               const std::vector<engine::world::AtlasTile>& atlasTiles,
                               const std::array<gl::SRGBA8, 256>& palette)
{
  for(const auto& quad : mesh.textured_rectangles)
  {
    const auto& tile = atlasTiles.at(quad.tileId.get());

    const auto firstVertex = m_vertices.size();

    bool useQuadHandling = isDistortedQuad(quad.vertices[0].from(mesh.vertices).toRenderSystem(),
                                           quad.vertices[1].from(mesh.vertices).toRenderSystem(),
                                           quad.vertices[2].from(mesh.vertices).toRenderSystem(),
                                           quad.vertices[3].from(mesh.vertices).toRenderSystem());

    for(int i = 0; i < 4; ++i)
    {
      RenderVertex iv{};
      if(useQuadHandling)
      {
        iv.isQuad = 1;
        iv.quadVert1 = quad.vertices[0].from(mesh.vertices).toRenderSystem();
        iv.quadVert2 = quad.vertices[1].from(mesh.vertices).toRenderSystem();
        iv.quadVert3 = quad.vertices[2].from(mesh.vertices).toRenderSystem();
        iv.quadVert4 = quad.vertices[3].from(mesh.vertices).toRenderSystem();
        iv.quadUv12 = glm::vec4{tile.uvCoordinates[0], tile.uvCoordinates[1]};
        iv.quadUv34 = glm::vec4{tile.uvCoordinates[2], tile.uvCoordinates[3]};
      }

      if(mesh.normals.empty())
        iv.color = glm::vec4(glm::vec3{toBrightness(quad.vertices[i].from(mesh.vertex_shades)).get()}, 1.0f);

      if(mesh.isFlatShaded() || mesh.normals.empty()
         || quad.vertices[i].from(mesh.normals) == core::TRVec{0_len, 0_len, 0_len})
      {
        if(i <= 2)
        {
          static const std::array<int, 3> indices{0, 1, 2};
          iv.normal = engine::world::generateNormal(quad.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                                    quad.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                                    quad.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
        }
        else
        {
          static const std::array<int, 3> indices{0, 2, 3};
          iv.normal = engine::world::generateNormal(quad.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                                    quad.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                                    quad.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
        }
      }
      else
      {
        iv.normal = quad.vertices[i].from(mesh.normals).toRenderSystem();
      }

      iv.position = quad.vertices[i].from(mesh.vertices).toRenderSystem();
      iv.uv = glm::vec3{tile.uvCoordinates[i], tile.textureKey.tileAndFlag & loader::file::TextureIndexMask};
      m_vertices.emplace_back(iv);
    }

    for(size_t i : {0, 1, 2, 0, 2, 3})
    {
      // cppcheck-suppress useStlAlgorithm
      m_indices.emplace_back(gsl::narrow<IndexType>(firstVertex + i));
    }
  }
  for(const auto& quad : mesh.colored_rectangles)
  {
    const auto color = glm::vec4{palette.at(quad.tileId.get() & 0xffu).channels} / 255.0f;

    const auto firstVertex = m_vertices.size();
    for(int i = 0; i < 4; ++i)
    {
      RenderVertex iv{};
      iv.position = quad.vertices[i].from(mesh.vertices).toRenderSystem();
      iv.uv = glm::vec3{0, 0, -1};
      iv.color = color;
      if(mesh.normals.empty())
        iv.color *= toBrightness(quad.vertices[i].from(mesh.vertex_shades)).get();

      if(mesh.isFlatShaded() || mesh.normals.empty()
         || quad.vertices[i].from(mesh.normals) == core::TRVec{0_len, 0_len, 0_len})
      {
        if(i <= 2)
        {
          static const std::array<int, 3> indices{0, 1, 2};
          iv.normal = engine::world::generateNormal(quad.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                                    quad.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                                    quad.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
        }
        else
        {
          static const std::array<int, 3> indices{0, 2, 3};
          iv.normal = engine::world::generateNormal(quad.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                                    quad.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                                    quad.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
        }
      }
      else
      {
        iv.normal = quad.vertices[i].from(mesh.normals).toRenderSystem();
      }
      m_vertices.emplace_back(iv);
    }
    for(size_t i : {0, 1, 2, 0, 2, 3})
    {
      // cppcheck-suppress useStlAlgorithm
      m_indices.emplace_back(gsl::narrow<IndexType>(firstVertex + i));
    }
  }

  for(const auto& tri : mesh.textured_triangles)
  {
    const auto& tile = atlasTiles.at(tri.tileId.get());

    for(int i = 0; i < 3; ++i)
    {
      RenderVertex iv{};
      iv.position = tri.vertices[i].from(mesh.vertices).toRenderSystem();
      iv.uv = glm::vec3{tile.uvCoordinates[i], tile.textureKey.tileAndFlag & loader::file::TextureIndexMask};
      if(mesh.normals.empty())
        iv.color = glm::vec4{glm::vec3{toBrightness(tri.vertices[i].from(mesh.vertex_shades)).get()}, 1.0f};

      if(mesh.isFlatShaded() || mesh.normals.empty()
         || tri.vertices[i].from(mesh.normals) == core::TRVec{0_len, 0_len, 0_len})
      {
        static const std::array<int, 3> indices{0, 1, 2};
        iv.normal = engine::world::generateNormal(tri.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                                  tri.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                                  tri.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
      }
      else
      {
        iv.normal = tri.vertices[i].from(mesh.normals).toRenderSystem();
      }
      m_indices.emplace_back(gsl::narrow<IndexType>(m_vertices.size()));
      m_vertices.emplace_back(iv);
    }
  }

  for(const auto& tri : mesh.colored_triangles)
  {
    const auto color = glm::vec4{palette.at(tri.tileId.get() & 0xffu).channels} / 255.0f;

    for(int i = 0; i < 3; ++i)
    {
      RenderVertex iv{};
      iv.position = tri.vertices[i].from(mesh.vertices).toRenderSystem();
      iv.uv = glm::vec3{0, 0, -1};
      iv.color = color;
      if(mesh.normals.empty())
        iv.color *= glm::vec4{glm::vec3{toBrightness(tri.vertices[i].from(mesh.vertex_shades)).get()}, 1.0f};

      if(mesh.isFlatShaded() || mesh.normals.empty()
         || tri.vertices[i].from(mesh.normals) == core::TRVec{0_len, 0_len, 0_len})
      {
        static const std::array<int, 3> indices{0, 1, 2};
        iv.normal = engine::world::generateNormal(tri.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                                  tri.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                                  tri.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
      }
      else
      {
        iv.normal = tri.vertices[i].from(mesh.normals).toRenderSystem();
      }
      m_indices.emplace_back(gsl::narrow<IndexType>(m_vertices.size()));
      m_vertices.emplace_back(iv);
    }
  }
}

gsl::not_null<std::shared_ptr<render::scene::Mesh>> RenderMeshDataCompositor::toMesh(
  render::scene::MaterialManager& materialManager, bool skeletal, const std::string& label)
{
  auto vb = std::make_shared<gl::VertexBuffer<RenderMeshData::RenderVertex>>(RenderMeshData::RenderVertex::getLayout(),
                                                                             label);
  vb->setData(m_vertices, gl::api::BufferUsage::StaticDraw);

#ifndef NDEBUG
  for(auto idx : m_indices)
  {
    BOOST_ASSERT(idx < m_vertices.size());
  }
#endif
  auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<RenderMeshData::IndexType>>(label);
  indexBuffer->setData(m_indices, gl::api::BufferUsage::StaticDraw);

  const auto material = materialManager.getGeometry(false, skeletal, false);
  const auto materialCSMDepthOnly = materialManager.getCSMDepthOnly(skeletal);
  const auto materialDepthOnly = materialManager.getDepthOnly(skeletal);

  auto va = std::make_shared<gl::VertexArray<RenderMeshData::IndexType, RenderMeshData::RenderVertex>>(
    indexBuffer,
    vb,
    std::vector<const gl::Program*>{&material->getShaderProgram()->getHandle(),
                                    &materialDepthOnly->getShaderProgram()->getHandle(),
                                    &materialCSMDepthOnly->getShaderProgram()->getHandle()},
    label);
  auto mesh = std::make_shared<render::scene::MeshImpl<RenderMeshData::IndexType, RenderMeshData::RenderVertex>>(
    va, gl::api::PrimitiveType::Triangles);
  mesh->getMaterialGroup()
    .set(render::scene::RenderMode::Full, material)
    .set(render::scene::RenderMode::DepthOnly, materialDepthOnly)
    .set(render::scene::RenderMode::CSMDepthOnly, materialCSMDepthOnly);
  mesh->getRenderState().setDepthTest(true);
  mesh->getRenderState().setDepthWrite(true);
  mesh->getRenderState().setDepthFunction(gl::api::DepthFunction::Less);

  return mesh;
}
} // namespace engine::world
