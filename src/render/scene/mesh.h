#pragma once

#include "rendercontext.h"
#include "renderable.h"

#include "render/gl/renderstate.h"
#include "render/gl/vertexarray.h"

namespace render
{
namespace scene
{
class Material;


class Mesh : public Renderable
{
public:
    using MaterialParameterSetter = void(const Node& node, Material& material);

    explicit Mesh(std::shared_ptr<gl::VertexArray> vao, GLenum mode = GL_TRIANGLES);

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

    void render(RenderContext& context) override;

    void registerMaterialParameterSetter(const std::function<MaterialParameterSetter>& setter)
    {
        m_materialParameterSetters.emplace_back( setter );
    }

    const auto& getVAO() const
    {
        return m_vao;
    }

private:
    std::shared_ptr<Material> m_material;

    std::vector<std::function<MaterialParameterSetter>> m_materialParameterSetters;

    gsl::not_null<std::shared_ptr<gl::VertexArray>> m_vao;

    const GLenum m_mode;
};


extern gsl::not_null<std::shared_ptr<Mesh>> createQuadFullscreen(float width,
                                                                 float height,
                                                                 const gl::Program& program,
                                                                 bool invertY = false);

}
}
