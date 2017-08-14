#pragma once

#include "RenderContext.h"

#include "gl/vertexarray.h"


namespace gameplay
{
    class Material;


    class MeshPart
    {
    public:
        using MaterialParameterSetter = void(Material& material);

        explicit MeshPart(const std::shared_ptr<gl::VertexArray>& vao);

        ~MeshPart();


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
            m_materialParameterSetters.emplace_back(setter);
        }


        const std::shared_ptr<gl::VertexArray>& getVao() const
        {
            return m_vao;
        }


    private:
        MeshPart(const MeshPart& copy) = delete;

        bool drawWireframe() const;

        std::shared_ptr<Material> m_material;

        std::vector<std::function<MaterialParameterSetter>> m_materialParameterSetters;

        std::shared_ptr<gl::VertexArray> m_vao;
    };
}
