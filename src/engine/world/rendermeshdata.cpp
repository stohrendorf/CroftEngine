#include "rendermeshdata.h"

#include "core/containeroffset.h"
#include "core/genericvec.h"
#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/world/atlastile.h"
#include "engine/world/util.h"
#include "loader/file/datatypes.h"
#include "loader/file/mesh.h"
#include "loader/file/primitives.h"
#include "loader/file/texture.h"
#include "render/material/material.h"
#include "render/material/materialgroup.h"
#include "render/material/materialmanager.h"
#include "render/material/rendermode.h"
#include "render/material/shaderprogram.h"
#include "render/scene/mesh.h"

#include <array>
#include <boost/assert.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <gl/buffer.h>
#include <gl/constants.h>
#include <gl/pixel.h>
#include <gl/renderstate.h>
#include <gl/vertexarray.h>
#include <gl/vertexbuffer.h>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

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

    const bool useQuadHandling = isDistortedQuad(quad.vertices[0].from(mesh.vertices).toRenderSystem(),
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

      const auto& quadVertex = quad.vertices[i];
      if(mesh.normals.empty())
        iv.color = glm::vec4(glm::vec3{toBrightness(quadVertex.from(mesh.vertex_shades)).get()}, 1.0f);

      if(mesh.isFlatShaded() || mesh.normals.empty()
         || quadVertex.from(mesh.normals) == core::TRVec{0_len, 0_len, 0_len})
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
        iv.normal = quadVertex.from(mesh.normals).toRenderSystem();
      }

      iv.position = quadVertex.from(mesh.vertices).toRenderSystem();
      iv.uv = glm::vec3{tile.uvCoordinates[i], tile.textureKey.atlasIdAndFlag & loader::file::AtlasIdMask};
      m_vertices.emplace_back(iv);
    }

    for(const size_t i : {0, 1, 2, 0, 2, 3})
    {
      const auto idx = gsl::narrow<IndexType>(firstVertex + i);
      if(tile.isOpaque())
        m_opaqueIndices.emplace_back(idx);
      else
        m_nonOpaqueIndices.emplace_back(idx);
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
    for(const size_t i : {0, 1, 2, 0, 2, 3})
    {
      const auto idx = gsl::narrow<IndexType>(firstVertex + i);
      if(color.a >= 1.0f)
        m_opaqueIndices.emplace_back(idx);
      else
        m_nonOpaqueIndices.emplace_back(idx);
    }
  }

  for(const auto& tri : mesh.textured_triangles)
  {
    const auto& tile = atlasTiles.at(tri.tileId.get());

    for(int i = 0; i < 3; ++i)
    {
      RenderVertex iv{};
      iv.position = tri.vertices[i].from(mesh.vertices).toRenderSystem();
      iv.uv = glm::vec3{tile.uvCoordinates[i], tile.textureKey.atlasIdAndFlag & loader::file::AtlasIdMask};
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
      const auto idx = gsl::narrow<IndexType>(m_vertices.size());
      if(tile.isOpaque())
        m_opaqueIndices.emplace_back(idx);
      else
        m_nonOpaqueIndices.emplace_back(idx);
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
      const auto idx = gsl::narrow<IndexType>(m_vertices.size());
      if(color.a >= 1.0f)
        m_opaqueIndices.emplace_back(idx);
      else
        m_nonOpaqueIndices.emplace_back(idx);
      m_vertices.emplace_back(iv);
    }
  }
}

gslu::nn_shared<render::scene::Mesh>
  RenderMeshDataCompositor::toMesh(render::material::MaterialManager& materialManager,
                                   bool skeletal,
                                   bool shadowCaster,
                                   const std::function<bool()>& smooth,
                                   const std::function<int32_t()>& lightingMode,
                                   const std::string& label)
{
  auto vb = gsl::make_shared<gl::VertexBuffer<RenderMeshData::RenderVertex>>(
    RenderMeshData::RenderVertex::getLayout(), label + gl::VboSuffix, gl::api::BufferUsage::StaticDraw, m_vertices);

#ifndef NDEBUG
  for(auto idx : m_opaqueIndices)
  {
    BOOST_ASSERT(idx < m_vertices.size());
  }
  for(auto idx : m_nonOpaqueIndices)
  {
    BOOST_ASSERT(idx < m_vertices.size());
  }
#endif
  auto indexBufferOpaque = gsl::make_shared<gl::ElementArrayBuffer<RenderMeshData::IndexType>>(
    label + gl::IndexBufferSuffix, gl::api::BufferUsage::StaticDraw, m_opaqueIndices);
  auto indexBufferNonOpaque = gsl::make_shared<gl::ElementArrayBuffer<RenderMeshData::IndexType>>(
    label + gl::IndexBufferSuffix, gl::api::BufferUsage::StaticDraw, m_nonOpaqueIndices);

  const auto materialOpaque = materialManager.getGeometry(false, skeletal, false, true, smooth, lightingMode);
  const auto materialNonOpaque = materialManager.getGeometry(false, skeletal, false, false, smooth, lightingMode);
  const auto materialCSMDepthOnly = materialManager.getCSMDepthOnly(skeletal, smooth);
  const auto materialDepthOnly = materialManager.getDepthOnly(skeletal, smooth);

  auto vaoOpaque = gsl::make_shared<gl::VertexArray<RenderMeshData::IndexType, RenderMeshData::RenderVertex>>(
    indexBufferOpaque,
    vb,
    std::vector<const gl::Program*>{&materialOpaque->getShaderProgram()->getHandle(),
                                    &materialDepthOnly->getShaderProgram()->getHandle(),
                                    &materialCSMDepthOnly->getShaderProgram()->getHandle()},
    label + "-opaque" + gl::VaoSuffix);
  auto vaoNonOpaque = gsl::make_shared<gl::VertexArray<RenderMeshData::IndexType, RenderMeshData::RenderVertex>>(
    indexBufferNonOpaque,
    vb,
    std::vector<const gl::Program*>{&materialNonOpaque->getShaderProgram()->getHandle(),
                                    &materialDepthOnly->getShaderProgram()->getHandle(),
                                    &materialCSMDepthOnly->getShaderProgram()->getHandle()},
    label + "-nonopaque" + gl::VaoSuffix);
  auto mesh = gsl::make_shared<render::scene::MeshImpl<RenderMeshData::IndexType, RenderMeshData::RenderVertex>>(
    vaoOpaque, vaoNonOpaque, gl::api::PrimitiveType::Triangles);

  mesh->getMaterialGroup()
    .set(render::material::RenderMode::FullOpaque, materialOpaque)
    .set(render::material::RenderMode::FullNonOpaque, materialNonOpaque)
    .set(render::material::RenderMode::DepthOnly, materialDepthOnly);
  if(shadowCaster)
  {
    mesh->getMaterialGroup().set(render::material::RenderMode::CSMDepthOnly, materialCSMDepthOnly);
  }

  mesh->getRenderState().setDepthTest(true);
  mesh->getRenderState().setDepthWrite(true);
  mesh->getRenderState().setDepthFunction(gl::api::DepthFunction::Less);

  return mesh;
}
} // namespace engine::world
