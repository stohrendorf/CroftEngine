#pragma once

#include "Mesh.h"
#include "VertexAttributeBinding.h"


namespace gameplay
{
    class Material;

    /**
     * Defines a part of a mesh describing the way the
     * mesh's vertices are connected together.
     */
    class MeshPart
    {
        friend class Mesh;
        friend class Model;

    public:
        explicit MeshPart();

        /**
         * Destructor.
         */
        ~MeshPart();

        /**
         * Gets the type of primitive to define how the indices are connected.
         *
         * @return The type of primitive.
         */
        Mesh::PrimitiveType getPrimitiveType() const;

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
        Mesh::IndexFormat getIndexFormat() const;

        /**
         * Returns a handle to the index buffer for the mesh part.
         *
         * @return The index buffer object handle.
         */
        IndexBufferHandle getIndexBuffer() const;

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
        void setIndexData(const void* indexData, size_t indexStart, size_t indexCount);

        const std::shared_ptr<VertexAttributeBinding>& getVaBinding() const
        {
            return _vaBinding;
        }

        void setMaterial(const std::shared_ptr<Material>& material);

    private:
        MeshPart(const MeshPart& copy) = delete;

        /**
         * Creates a mesh part for the specified mesh.
         *
         * @param mesh The mesh that this is part of.
         * @param meshIndex The index of the part within the mesh.
         * @param primitiveType The primitive type.
         * @param indexFormat The index format.
         * @param indexCount The number of indices.
         * @param dynamic true if the part if dynamic; false otherwise.
         */
        static std::shared_ptr<MeshPart> create(const std::weak_ptr<Mesh>& mesh, Mesh::PrimitiveType primitiveType, Mesh::IndexFormat indexFormat, size_t indexCount, bool dynamic = false);

        std::weak_ptr<Mesh> _mesh{};
        Mesh::PrimitiveType _primitiveType = Mesh::TRIANGLES;
        Mesh::IndexFormat _indexFormat{};
        size_t _indexCount = 0;
        IndexBufferHandle _indexBuffer = 0;
        bool _dynamic = false;
        std::shared_ptr<VertexAttributeBinding> _vaBinding;
        std::shared_ptr<Material> _material;
    };
}
