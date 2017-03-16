#pragma once

#include "Mesh.h"
#include "Font.h"
#include "RenderContext.h"

#include "gl/indexbuffer.h"
#include "gl/vertexarray.h"


namespace gameplay
{
    class Material;


    /**
     * Defines a part of a mesh describing the way the
     * mesh's vertices are connected together.
     */
    class MeshPart : public gl::IndexBuffer
    {
        friend class Mesh;
        friend class Model;

    public:
        using MaterialParameterSetter = void(Material& material);

        explicit MeshPart(const gsl::not_null<Mesh*>& mesh, PrimitiveType primitiveType, GLint indexFormat, size_t indexCount, bool dynamic = false);

        /**
         * Destructor.
         */
        ~MeshPart();

        /**
         * Gets the type of primitive to define how the indices are connected.
         *
         * @return The type of primitive.
         */
        PrimitiveType getPrimitiveType() const;

        /**
         * Gets the number of indices in the part.
         *
         * @return The number of indices in the part.
         */
        size_t getIndexCount() const;

        /**
         * Returns the format of the part indices.
         *
         * @return The part index format.
         */
        GLint getIndexFormat() const;

        /**
         * Determines if the indices are dynamic.
         *
         * @return true if the part is dynamic; false otherwise.
         */
        bool isDynamic() const;

        /**
         * Sets the specified index data into the mapped index buffer.
         *
         * @param indexData The index data to be set.
         * @param indexStart The index to start from.
         * @param indexCount The number of indices to be set.
         * @script{ignore}
         */
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
        PrimitiveType _primitiveType = PrimitiveType::TRIANGLES;
        GLint _indexFormat{};
        size_t _indexCount = 0;
        bool _dynamic = false;
        std::shared_ptr<Material> _material;
        std::vector<std::function<MaterialParameterSetter>> _materialParameterSetters;
        mutable std::shared_ptr<gl::VertexArray> m_vao = nullptr;
    };
}
