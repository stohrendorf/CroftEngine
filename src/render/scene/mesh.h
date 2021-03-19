#pragma once

#include "materialgroup.h"
#include "materialparameteroverrider.h"
#include "renderable.h"

#include <gl/api/gl.hpp>
#include <gl/soglb_fwd.h>

namespace render::scene
{
class RenderContext;

class Mesh
    : public Renderable
    , public MaterialParameterOverrider
{
public:
  explicit Mesh(gl::api::PrimitiveType primitiveType = gl::api::PrimitiveType::Triangles)
      : m_primitiveType{primitiveType}
  {
  }

  ~Mesh() override;

  Mesh(const Mesh&) = delete;
  Mesh(Mesh&&) = delete;
  Mesh& operator=(Mesh&&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  [[nodiscard]] const auto& getMaterialGroup() const
  {
    return m_materialGroup;
  }

  [[nodiscard]] auto& getMaterialGroup()
  {
    return m_materialGroup;
  }

  bool render(RenderContext& context) final;

private:
  MaterialGroup m_materialGroup{};
  const gl::api::PrimitiveType m_primitiveType{};

  virtual void drawIndexBuffer(gl::api::PrimitiveType primitiveType) = 0;
};

template<typename IndexT, typename... VertexTs>
class MeshImpl : public Mesh
{
public:
  explicit MeshImpl(std::shared_ptr<gl::VertexArray<IndexT, VertexTs...>> vao,
                    gl::api::PrimitiveType primitiveType = gl::api::PrimitiveType::Triangles)
      : Mesh{primitiveType}
      , m_vao{std::move(vao)}
  {
  }

  ~MeshImpl() override = default;

  MeshImpl(const MeshImpl&) = delete;
  MeshImpl(MeshImpl&&) = delete;
  MeshImpl& operator=(MeshImpl&&) = delete;
  MeshImpl& operator=(const MeshImpl&) = delete;

private:
  gsl::not_null<std::shared_ptr<gl::VertexArray<IndexT, VertexTs...>>> m_vao;

  void drawIndexBuffer(gl::api::PrimitiveType primitiveType) override
  {
    m_vao->drawIndexBuffer(primitiveType);
  }
};

extern gsl::not_null<std::shared_ptr<Mesh>>
  createQuadFullscreen(float width, float height, const gl::Program& program, bool invertY = false);
} // namespace render::scene
