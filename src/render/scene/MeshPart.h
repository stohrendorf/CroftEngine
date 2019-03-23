#pragma once

#include "RenderContext.h"
#include "Drawable.h"

#include "render/gl/renderstate.h"
#include "render/gl/vertexarray.h"

namespace render
{
namespace scene
{
class Material;


class MeshPart : public Drawable
{
public:
    using MaterialParameterSetter = void(const Node& node, Material& material);

    explicit MeshPart(std::shared_ptr<gl::VertexArray> vao, GLenum mode = GL_TRIANGLES);

    ~MeshPart() override;

    MeshPart(const MeshPart&) = delete;

    MeshPart(MeshPart&&) = delete;

    MeshPart& operator=(MeshPart&&) = delete;

    MeshPart& operator=(const MeshPart&) = delete;

    void setMaterial(const std::shared_ptr<Material>& material)
    {
        m_material = material;
    }

    const std::shared_ptr<Material>& getMaterial() const
    {
        return m_material;
    }

    void draw(RenderContext& context) override;

    void registerMaterialParameterSetter(const std::function<MaterialParameterSetter>& setter)
    {
        m_materialParameterSetters.emplace_back( setter );
    }

private:
    std::shared_ptr<Material> m_material;

    std::vector<std::function<MaterialParameterSetter>> m_materialParameterSetters;

    std::shared_ptr<gl::VertexArray> m_vao;

    const GLenum m_mode;
};
}
}
