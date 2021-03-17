#pragma once

#include "mesh.h"
#include "render/scene/names.h"

#include <gl/pixel.h>
#include <gl/vertexbuffer.h>
#include <glm/common.hpp>

namespace render::scene
{
class MaterialManager;
class Mesh;
} // namespace render::scene

namespace loader::file
{
struct Mesh;
struct Palette;
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
    glm::vec2 uv;
    glm::int32_t textureIndex{-1};
    glm::int32_t boneIndex{-1};

    static const gl::VertexFormat<RenderVertex>& getFormat()
    {
      static const gl::VertexFormat<RenderVertex> format{{VERTEX_ATTRIBUTE_POSITION_NAME, &RenderVertex::position},
                                                         {VERTEX_ATTRIBUTE_NORMAL_NAME, &RenderVertex::normal},
                                                         {VERTEX_ATTRIBUTE_COLOR_NAME, &RenderVertex::color},
                                                         {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &RenderVertex::uv},
                                                         {VERTEX_ATTRIBUTE_TEXINDEX_NAME, &RenderVertex::textureIndex},
                                                         {VERTEX_ATTRIBUTE_BONE_INDEX_NAME, &RenderVertex::boneIndex}};

      return format;
    }
  };

  explicit RenderMeshData(const loader::file::Mesh& mesh,
                          const std::vector<engine::world::AtlasTile>& atlasTiles,
                          const std::array<gl::SRGBA8, 256>& palette);

  [[nodiscard]] const auto& getVertices() const
  {
    return m_vertices;
  }

  [[nodiscard]] const auto& getIndices() const
  {
    return m_indices;
  }

private:
  std::vector<RenderVertex> m_vertices{};
  std::vector<IndexType> m_indices{};
};

class RenderMeshDataCompositor final
{
public:
  void append(const RenderMeshData& data)
  {
    const auto vertexOffset = gsl::narrow<RenderMeshData::IndexType>(m_vertices.size());
    for(auto v : data.getVertices())
    {
      v.boneIndex = m_boneIndex;
      m_vertices.emplace_back(v);
    }

    for(auto i : data.getIndices())
    {
      // cppcheck-suppress useStlAlgorithm
      m_indices.emplace_back(gsl::narrow<RenderMeshData::IndexType>(i + vertexOffset));
    }

    ++m_boneIndex;
  }

  void appendEmpty()
  {
    ++m_boneIndex;
  }

  gsl::not_null<std::shared_ptr<render::scene::Mesh>>
    toMesh(render::scene::MaterialManager& materialManager, bool skeletal, const std::string& label);

  [[nodiscard]] bool empty() const
  {
    return m_vertices.empty() || m_indices.empty();
  }

private:
  std::vector<RenderMeshData::RenderVertex> m_vertices{};
  std::vector<RenderMeshData::IndexType> m_indices{};
  glm::int32_t m_boneIndex = 0;
};
} // namespace engine::world
