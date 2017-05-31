#pragma once

#include "Mesh.h"
#include "RenderContext.h"

#include "gl/indexbuffer.h"
#include "gl/vertexarray.h"


namespace gameplay
{
    class Material;


    class MeshPart
    {
        friend class Mesh;

        friend class Model;

    public:
        using MaterialParameterSetter = void(Material& material);

        explicit MeshPart(const std::shared_ptr<gl::VertexArray>& vao);

        ~MeshPart();


        void setMaterial(const std::shared_ptr<Material>& material)
        {
            _material = material;
        }


        const std::shared_ptr<Material>& getMaterial() const
        {
            return _material;
        }


        void draw(RenderContext& context) const;


        void registerMaterialParameterSetter(const std::function<MaterialParameterSetter>& setter)
        {
            _materialParameterSetters.emplace_back(setter);
        }


        const std::shared_ptr<gl::VertexArray>& getVao() const
        {
            return m_vao;
        }


    private:
        MeshPart(const MeshPart& copy) = delete;

        bool drawWireframe() const;

        std::shared_ptr<Material> _material;

        std::vector<std::function<MaterialParameterSetter>> _materialParameterSetters;

        std::shared_ptr<gl::VertexArray> m_vao;
    };
}
