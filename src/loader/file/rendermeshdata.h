#pragma once

#include "datatypes.h"
#include "mesh.h"
#include "util.h"

#include <gl/vertexarray.h>

namespace loader::file
{
class RenderMeshData final
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

  std::vector<RenderVertex> m_vertices{};
  std::vector<IndexType> m_indices{};

public:
  explicit RenderMeshData(const Mesh& mesh, const std::vector<TextureTile>& textureTiles, const Palette& palette);

  gsl::not_null<std::shared_ptr<render::scene::Mesh>>
    toMesh(const gsl::not_null<std::shared_ptr<render::scene::Material>>& materialFull,
           const gsl::not_null<std::shared_ptr<render::scene::Material>>& materialDepthOnly,
           const gsl::not_null<std::shared_ptr<render::scene::Material>>& materialCSMDepthOnly,
           const std::string& label);
};
} // namespace loader::file