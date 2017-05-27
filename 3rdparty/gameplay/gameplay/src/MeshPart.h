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

        explicit MeshPart(const gsl::not_null<Mesh*>& mesh, GLenum primitiveType, GLint indexFormat, size_t indexCount, bool dynamic = false);

        ~MeshPart();

        GLenum getPrimitiveType() const;

        size_t getIndexCount() const;

        GLint getIndexFormat() const;

        bool isDynamic() const;

        void setIndexData(const gsl::not_null<const void*>& indexData, size_t indexStart, size_t indexCount);

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
        GLenum _primitiveType = GL_TRIANGLES;
        GLint _indexFormat{};
        size_t _indexCount = 0;
        bool _dynamic = false;
        std::shared_ptr<Material> _material;
        std::vector<std::function<MaterialParameterSetter>> _materialParameterSetters;
        mutable std::shared_ptr<gl::VertexArray> m_vao = nullptr;
    };
}
