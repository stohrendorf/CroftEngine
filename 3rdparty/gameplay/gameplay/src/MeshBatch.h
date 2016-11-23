#pragma once

#include "Mesh.h"
#include "VertexAttributeBinding.h"


namespace gameplay
{
    class Material;
    class RenderContext;

    /**
     * Defines a class for rendering multiple mesh into a single draw call on the graphics device.
     */
    class MeshBatch
    {
    public:

        /**
        * Creates a new mesh batch.
        *
        * @param vertexFormat The format of vertices in the new batch.
        * @param primitiveType The type of primitives that will be added to the batch.
        * @param material Material to be used for drawing the batch.
        *
        * @return A new mesh batch.
        * @script{create}
        */
        MeshBatch(const VertexFormat& vertexFormat, Mesh::PrimitiveType primitiveType, const std::shared_ptr<Material>& material);

        /**
         * Destructor.
         */
        ~MeshBatch();

        /**
         * Returns the material for this mesh batch.
         *
         * @return The material used to draw the batch.
         */
        inline const std::shared_ptr<Material>& getMaterial() const;

        /**
         * Adds a group of primitives to the batch.
         *
         * The vertex list passed in should be a pointer of structs, where the struct T represents
         * the format of a single vertex (e.g. {x,y,z,u,v}).
         *
         * If the batch was created with 'indexed' set to true, then valid index data should be
         * passed in this method. However, if 'indexed' was set to false, the indices and indexCount
         * parameters can be omitted since only vertex data will be used.
         *
         * If the batch created to draw triangle strips, this method assumes that separate calls to
         * add specify separate triangle strips. In this case, this method will automatically stitch
         * separate triangle strips together using degenerate (zero-area) triangles.
         *
         * @param vertices Array of vertices.
         * @param vertexCount Number of vertices.
         * @param indices Array of indices into the vertex array (should be NULL for non-indexed batches).
         */
        template<class T>
        void add(const T* vertices, size_t vertexCount, const std::vector<uint16_t>& indices = {});

        /**
         * Starts batching.
         *
         * This method should be called before calling add() to add primitives to the batch.
         * After all primitives have been added to the batch, call the finish() method to
         * complete the batch.
         *
         * Calling this method will clear any primitives currently in the batch and set the
         * position of the batch back to the beginning.
         */
        void start();

        /**
        * Determines if the batch has been started and not yet finished.
        */
        bool isStarted() const;

        /**
         * Indicates that batching is complete and prepares the batch for drawing.
         */
        void finish();

        /**
         * Draws the primitives currently in batch.
         */
        void draw(RenderContext& context);

    private:

        MeshBatch(const MeshBatch& copy) = delete;
        MeshBatch& operator=(const MeshBatch&) = delete;

        void add(const void* vertices, size_t vertexCount, const std::vector<uint16_t>& indices);

        const VertexFormat m_vertexFormat;
        Mesh::PrimitiveType m_primitiveType;
        std::shared_ptr<Material> m_material;
        std::vector<uint8_t> m_vertices;
        std::vector<uint16_t> m_indices;
        bool m_started;
    };


    const std::shared_ptr<Material>& MeshBatch::getMaterial() const
    {
        return m_material;
    }


    template<class T>
    void MeshBatch::add(const T* vertices, size_t vertexCount, const std::vector<uint16_t>& indices)
    {
        BOOST_ASSERT(sizeof(T) == m_vertexFormat.getVertexSize());
        add(static_cast<const void*>(vertices), vertexCount, indices);
    }
}
