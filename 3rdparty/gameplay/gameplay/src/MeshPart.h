#pragma once

#include "Mesh.h"
#include "RenderContext.h"

#include "gl/indexbuffer.h"
#include "gl/vertexarray.h"


namespace gameplay
{
    class Material;


    class MeshPart : public gl::IndexBuffer
    {
        friend class Mesh;

        friend class Model;

    public:
        using MaterialParameterSetter = void(Material& material);

        explicit MeshPart(const gsl::not_null<Mesh*>& mesh, GLint indexFormat);

        ~MeshPart();


        GLint getIndexFormat() const
        {
            return _indexFormat;
        }


        void setMaterial(const std::shared_ptr<Material>& material);


        const std::shared_ptr<Material>& getMaterial() const
        {
            return _material;
        }


        void draw(RenderContext& context) const;


        void registerMaterialParameterSetter(const std::function<MaterialParameterSetter>& setter)
        {
            _materialParameterSetters.emplace_back(setter);
        }


    private:
        MeshPart(const MeshPart& copy) = delete;

        bool drawWireframe() const;

        const gsl::not_null<Mesh*> _mesh;

        GLint _indexFormat{};

        std::shared_ptr<Material> _material;

        std::vector<std::function<MaterialParameterSetter>> _materialParameterSetters;

        mutable std::shared_ptr<gl::VertexArray> m_vao = nullptr;
    };
}
