#pragma once

#include "RenderContext.h"
#include "RenderState.h"

#include "gl/vertexarray.h"

namespace gameplay
{
class Material;


class MeshPart
{
public:
    using MaterialParameterSetter = void(const Node& node, Material& material);

    explicit MeshPart(const std::shared_ptr<gl::VertexArray>& vao);

    ~MeshPart();

    MeshPart(const MeshPart& copy) = delete;

    void setMaterial(const std::shared_ptr<Material>& material)
    {
        m_material = material;
    }

    const std::shared_ptr<Material>& getMaterial() const
    {
        return m_material;
    }

    void draw(RenderContext& context) const;

    void registerMaterialParameterSetter(const std::function<MaterialParameterSetter>& setter)
    {
        m_materialParameterSetters.emplace_back( setter );
    }

    RenderState& getRenderState()
    {
        return m_renderState;
    }

private:
    RenderState m_renderState{};

    std::shared_ptr<Material> m_material;

    std::vector<std::function<MaterialParameterSetter>> m_materialParameterSetters;

    std::shared_ptr<gl::VertexArray> m_vao;
};
}
