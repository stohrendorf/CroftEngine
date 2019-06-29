#pragma once

#include "render/gl/renderstate.h"
#include "render/gl/vertexarray.h"
#include "renderable.h"
#include "rendercontext.h"

namespace render
{
namespace scene
{
class Material;

class Mesh : public Renderable
{
public:
    using MaterialUniformSetter = void(const Node& node, Material& material);

    explicit Mesh(::gl::PrimitiveType primitiveType = ::gl::PrimitiveType::Triangles)
        : m_primitiveType{primitiveType}
    {
    }

    ~Mesh() override;

    Mesh(const Mesh&) = delete;

    Mesh(Mesh&&) = delete;

    Mesh& operator=(Mesh&&) = delete;

    Mesh& operator=(const Mesh&) = delete;

    void setMaterial(const std::shared_ptr<Material>& material)
    {
        m_material = material;
    }

    const std::shared_ptr<Material>& getMaterial() const
    {
        return m_material;
    }

    void render(RenderContext& context) final;

    void registerMaterialUniformSetter(const std::function<MaterialUniformSetter>& setter)
    {
        m_materialUniformSetters.emplace_back(setter);
    }

private:
    std::shared_ptr<Material> m_material;

    std::vector<std::function<MaterialUniformSetter>> m_materialUniformSetters;

    const ::gl::PrimitiveType m_primitiveType;

    virtual void drawIndexBuffers(::gl::PrimitiveType primitiveType) = 0;
};

template<typename IndexT, typename... VertexTs>
class MeshImpl : public Mesh
{
public:
    explicit MeshImpl(std::shared_ptr<gl::VertexArray<IndexT, VertexTs...>> vao,
                      ::gl::PrimitiveType primitiveType = ::gl::PrimitiveType::Triangles)
        : Mesh{primitiveType}
        , m_vao{std::move(vao)}
    {
    }

    ~MeshImpl() override = default;

    MeshImpl(const MeshImpl&) = delete;

    MeshImpl(MeshImpl&&) = delete;

    MeshImpl& operator=(MeshImpl&&) = delete;

    MeshImpl& operator=(const MeshImpl&) = delete;

    const auto& getVAO() const
    {
        return m_vao;
    }

private:
    gsl::not_null<std::shared_ptr<gl::VertexArray<IndexT, VertexTs...>>> m_vao;

    void drawIndexBuffers(::gl::PrimitiveType primitiveType) override
    {
        m_vao->bind();

        for(const auto& buffer : m_vao->getIndexBuffers())
        {
            GL_ASSERT(
                ::gl::drawElements(primitiveType, buffer->size(), gl::TypeTraits<IndexT>::DrawElementsType, nullptr));
        }

        m_vao->unbind();
    }
};

extern gsl::not_null<std::shared_ptr<Mesh>>
    createQuadFullscreen(float width, float height, const gl::Program& program, bool invertY = false);
} // namespace scene
} // namespace render
