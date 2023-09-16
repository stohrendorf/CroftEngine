#pragma once

#include "render/material/materialgroup.h"
#include "render/material/materialparameteroverrider.h"
#include "renderable.h"
#include "translucency.h"

#include <gl/api/gl.hpp>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <gslu.h>
#include <memory>
#include <string>
#include <utility>

namespace render::material
{
class Material;
}

namespace render::scene
{
class RenderContext;
class Node;
enum class Translucency;

class Mesh
    : public Renderable
    , public material::MaterialParameterOverrider
{
public:
  explicit Mesh(gl::api::PrimitiveType primitiveType)
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

  void render(const Node* node, RenderContext& context) final;
  void render(const Node* node, RenderContext& context, gl::api::core::SizeType instanceCount) final;

  [[nodiscard]] auto getPrimitiveType() const
  {
    return m_primitiveType;
  }

private:
  material::MaterialGroup m_materialGroup{};
  gl::api::PrimitiveType m_primitiveType{};

  virtual void drawElements(Translucency translucencySelector) = 0;
  virtual void drawElements(Translucency translucencySelector, gl::api::core::SizeType instanceCount) = 0;
  [[nodiscard]] virtual bool empty(Translucency translucencySelector) const = 0;
};

template<typename IndexT, typename... VertexTs>
class MeshImpl : public Mesh
{
public:
  explicit MeshImpl(std::shared_ptr<gl::VertexArray<IndexT, VertexTs...>> vaoOpaque,
                    std::shared_ptr<gl::VertexArray<IndexT, VertexTs...>> vaoNonOpaque,
                    gl::api::PrimitiveType primitiveType)
      : Mesh{primitiveType}
      , m_vaoOpaque{std::move(vaoOpaque)}
      , m_vaoNonOpaque{std::move(vaoNonOpaque)}
  {
  }

  ~MeshImpl() override = default;

  MeshImpl(const MeshImpl&) = delete;
  MeshImpl(MeshImpl&&) = delete;
  MeshImpl& operator=(MeshImpl&&) = delete;
  MeshImpl& operator=(const MeshImpl&) = delete;

private:
  std::shared_ptr<gl::VertexArray<IndexT, VertexTs...>> m_vaoOpaque;
  std::shared_ptr<gl::VertexArray<IndexT, VertexTs...>> m_vaoNonOpaque;

  void drawElements(Translucency translucencySelector) override
  {
    switch(translucencySelector)
    {
    case Translucency::Opaque:
      if(m_vaoOpaque != nullptr)
        m_vaoOpaque->drawElements(getPrimitiveType());
      break;
    case Translucency::NonOpaque:
      if(m_vaoNonOpaque != nullptr)
        m_vaoNonOpaque->drawElements(getPrimitiveType());
      break;
    }
  }

  void drawElements(Translucency translucencySelector, gl::api::core::SizeType instanceCount) override
  {
    switch(translucencySelector)
    {
    case Translucency::Opaque:
      if(m_vaoOpaque != nullptr)
        m_vaoOpaque->drawElements(getPrimitiveType(), instanceCount);
      break;
    case Translucency::NonOpaque:
      if(m_vaoNonOpaque != nullptr)
        m_vaoNonOpaque->drawElements(getPrimitiveType(), instanceCount);
      break;
    }
  }

  bool empty(Translucency translucencySelector) const override
  {
    switch(translucencySelector)
    {
    case Translucency::Opaque:
      return m_vaoOpaque == nullptr || m_vaoOpaque->empty();
    case Translucency::NonOpaque:
      return m_vaoNonOpaque == nullptr || m_vaoNonOpaque->empty();
    }

    return true;
  }
};

extern gslu::nn_shared<Mesh> createScreenQuad(const glm::vec2& xy,
                                              const glm::vec2& size,
                                              const std::shared_ptr<material::Material>& material,
                                              Translucency spriteTranslucency,
                                              const std::string& label);

inline gslu::nn_shared<Mesh> createScreenQuad(const glm::vec2& size,
                                              const std::shared_ptr<material::Material>& material,
                                              Translucency spriteTranslucency,
                                              const std::string& label)
{
  return createScreenQuad({0, 0}, size, material, spriteTranslucency, label);
}

inline gslu::nn_shared<Mesh> createScreenQuad(const std::shared_ptr<material::Material>& material,
                                              Translucency spriteTranslucency,
                                              const std::string& label)
{
  return createScreenQuad({0, 0}, {0, 0}, material, spriteTranslucency, label);
}
} // namespace render::scene
