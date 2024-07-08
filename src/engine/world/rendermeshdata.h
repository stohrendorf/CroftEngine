#pragma once

#include "render/scene/names.h"

#include <array>
#include <cstdint>
#include <functional>
#include <gl/pixel.h>
#include <gl/vertexbuffer.h>
#include <glm/ext/scalar_int_sized.hpp>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <string>
#include <vector>

namespace render::material
{
class MaterialManager;
}

namespace render::scene
{
class Mesh;
} // namespace render::scene

namespace loader::file
{
struct Mesh;
} // namespace loader::file

namespace engine::world
{
struct AtlasTile;

class RenderMeshData final
{
public:
  using IndexType = uint16_t;

  struct RenderVertex
  {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 color{1.0f};
    glm::vec3 uv;
    glm::int32 boneIndex{-1};
    glm::int32 isQuad{0};
    glm::vec3 quadVert1;
    glm::vec3 quadVert2;
    glm::vec3 quadVert3;
    glm::vec3 quadVert4;
    glm::vec4 quadUv12;
    glm::vec4 quadUv34;
    glm::vec4 reflective{0, 0, 0, 0};

    static const gl::VertexLayout<RenderVertex>& getLayout()
    {
      static const gl::VertexLayout<RenderVertex> layout{
        {VERTEX_ATTRIBUTE_POSITION_NAME, &RenderVertex::position},
        {VERTEX_ATTRIBUTE_NORMAL_NAME, &RenderVertex::normal},
        {VERTEX_ATTRIBUTE_COLOR_NAME, &RenderVertex::color},
        {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &RenderVertex::uv},
        {VERTEX_ATTRIBUTE_BONE_INDEX_NAME, &RenderVertex::boneIndex},
        {VERTEX_ATTRIBUTE_IS_QUAD, &RenderVertex::isQuad},
        {VERTEX_ATTRIBUTE_QUAD_VERT1, &RenderVertex::quadVert1},
        {VERTEX_ATTRIBUTE_QUAD_VERT2, &RenderVertex::quadVert2},
        {VERTEX_ATTRIBUTE_QUAD_VERT3, &RenderVertex::quadVert3},
        {VERTEX_ATTRIBUTE_QUAD_VERT4, &RenderVertex::quadVert4},
        {VERTEX_ATTRIBUTE_QUAD_UV12, &RenderVertex::quadUv12},
        {VERTEX_ATTRIBUTE_QUAD_UV34, &RenderVertex::quadUv34},
        {VERTEX_ATTRIBUTE_REFLECTIVE_NAME, &RenderVertex::reflective},
      };

      return layout;
    }
  };

  explicit RenderMeshData(const loader::file::Mesh& mesh,
                          const std::vector<engine::world::AtlasTile>& atlasTiles,
                          const std::array<gl::SRGBA8, 256>& palette);

  [[nodiscard]] const auto& getVertices() const noexcept
  {
    return m_vertices;
  }

  [[nodiscard]] const auto& getOpaqueIndices() const noexcept
  {
    return m_opaqueIndices;
  }

  [[nodiscard]] const auto& getNonOpaqueIndices() const noexcept
  {
    return m_nonOpaqueIndices;
  }

private:
  std::vector<RenderVertex> m_vertices;
  std::vector<IndexType> m_opaqueIndices;
  std::vector<IndexType> m_nonOpaqueIndices;
};

class RenderMeshDataCompositor final
{
public:
  void append(const RenderMeshData& data, const gl::SRGBA8& reflective)
  {
    const auto vertexOffset = gsl::narrow<RenderMeshData::IndexType>(m_vertices.size());
    for(auto v : data.getVertices())
    {
      v.boneIndex = m_boneIndex;
      v.reflective = glm::vec4(reflective.channels) / 255.0f;
      m_vertices.emplace_back(v);
    }

    for(auto i : data.getOpaqueIndices())
    {
      // cppcheck-suppress useStlAlgorithm
      m_opaqueIndices.emplace_back(gsl::narrow<RenderMeshData::IndexType>(i + vertexOffset));
    }
    for(auto i : data.getNonOpaqueIndices())
    {
      // cppcheck-suppress useStlAlgorithm
      m_nonOpaqueIndices.emplace_back(gsl::narrow<RenderMeshData::IndexType>(i + vertexOffset));
    }

    ++m_boneIndex;
  }

  void appendEmpty() noexcept
  {
    ++m_boneIndex;
  }

  gslu::nn_shared<render::scene::Mesh> toMesh(render::material::MaterialManager& materialManager,
                                              bool skeletal,
                                              bool shadowCaster,
                                              const std::function<bool()>& smooth,
                                              const std::function<int32_t()>& lightingMode,
                                              const std::string& label);

  [[nodiscard]] bool empty() const noexcept
  {
    return m_vertices.empty() || (m_opaqueIndices.empty() && m_nonOpaqueIndices.empty());
  }

private:
  std::vector<RenderMeshData::RenderVertex> m_vertices;
  std::vector<RenderMeshData::IndexType> m_opaqueIndices;
  std::vector<RenderMeshData::IndexType> m_nonOpaqueIndices;
  glm::int32_t m_boneIndex = 0;
};
} // namespace engine::world
