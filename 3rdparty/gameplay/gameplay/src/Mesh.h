#pragma once

#include "VertexFormat.h"
#include "BoundingBox.h"

#include <memory>


namespace gameplay
{
    class MeshPart;
    class Model;


    /**
     * Defines a mesh supporting various vertex formats and 1 or more
     * MeshPart(s) to define how the vertices are connected.
     */
    class Mesh : public std::enable_shared_from_this<Mesh>
    {
        friend class Model;

    public:
        explicit Mesh(const VertexFormat& vertexFormat, size_t vertexCount, bool dynamic);


        /**
         * Defines supported index formats.
         */
        enum IndexFormat
        {
            INDEX8 = GL_UNSIGNED_BYTE,
            INDEX16 = GL_UNSIGNED_SHORT,
            INDEX32 = GL_UNSIGNED_INT
        };


        /**
         * Defines supported primitive types.
         */
        enum PrimitiveType
        {
            TRIANGLES = GL_TRIANGLES,
            TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
            LINES = GL_LINES,
            LINE_STRIP = GL_LINE_STRIP,
            POINTS = GL_POINTS
        };


        void rebuild(const float* vertexData, size_t vertexCount);

        /**
         * Creates a new textured 3D quad.
         *
         * The specified points should describe a triangle strip with the first 3 points
         * forming a triangle wound in counter-clockwise direction, with the second triangle
         * formed from the last three points in clockwise direction.
         *
         * @param p1 The first point.
         * @param p2 The second point.
         * @param p3 The third point.
         * @param p4 The fourth point.
         *
         * @return The created mesh.
         * @script{create}
         */
        static std::shared_ptr<Mesh> createQuad(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4);

        /**
         * Constructs a new textured 2D quad.
         *
         * @param x The x coordinate.
         * @param y The y coordinate.
         * @param width The width of the quad.
         * @param height The height of the quad.
         * @param s1 The S texture coordinate of the bottom left point.
         * @param t1 The T texture coordinate of the bottom left point.
         * @param s2 The S texture coordinate of the top right point.
         * @param t2 The T texture coordinate of the top right point.
         *
         * @return The newly created mesh.
         * @script{create}
         */
        static std::shared_ptr<Mesh> createQuad(float x, float y, float width, float height, float s1 = 0.0f, float t1 = 0.0f, float s2 = 1.0f, float t2 = 1.0f);

        /**
         * Creates a new full-screen 2D quad.
         *
         * The returned mesh's vertex format includes a 2-element (x,y) position
         * and a 2-element texture coordinate.
         *
         * This method returns a mesh describing a fullscreen quad using
         * normalized device coordinates for vertex positions.
         *
         * @return The newly created mesh.
         * @script{create}
         */
        static std::shared_ptr<Mesh> createQuadFullscreen();

        /**
         * Creates lines between 2 or more points passed in as a glm::vec3 array.
         *
         * The mesh contains only position data using lines to connect the vertices.
         * This is useful for drawing basic color elements into a scene.
         *
         * @param points The array of points.
         * @param pointCount The number of points.
         *
         * @return The newly created mesh.
         * @script{create}
         */
        static std::shared_ptr<Mesh> createLines(glm::vec3* points, size_t pointCount);

        /**
         * Creates a bounding box mesh when passed a BoundingBox.
         *
         * The mesh contains only position data using lines to connect the vertices.
         *
         * @param box The BoundingBox that will be used to create the mesh.
         *
         * @return The newly created bounding box mesh.
         * @script{create}
         */
        static std::shared_ptr<Mesh> createBoundingBox(const BoundingBox& box);

        /**
         * Gets the vertex format for the mesh.
         *
         * @return The vertex format.
         */
        const VertexFormat& getVertexFormat() const;

        /**
         * Gets the number of vertices in the mesh.
         *
         * @return The number of vertices in the mesh.
         */
        size_t getVertexCount() const;

        /**
         * Gets the size of a single vertex in the mesh.
         *
         * @return The size of 1 vertex in the mesh.
         */
        size_t getVertexSize() const;

        /**
         * Returns a handle to the vertex buffer for the mesh.
         *
         * @return The vertex buffer object handle.
         */
        const VertexBufferHandle& getVertexBuffer() const;

        /**
         * Determines if the mesh is dynamic.
         *
         * @return true if the mesh is dynamic; false otherwise.
         */
        bool isDynamic() const;

        /**
         * Returns the primitive type of the vertices in the mesh.
         *
         * The default primitive type for a Mesh is TRIANGLES.
         *
         * @return The primitive type.
         *
         * @see setPrimitiveType(PrimitiveType)
         */
        PrimitiveType getPrimitiveType() const;

        /**
         * Sets the primitive type for the vertices in the mesh.
         *
         * The primitive type for a Mesh is only meaningful for meshes that do not
         * have any MeshParts. When there are no MeshParts associated with a mesh,
         * the Mesh is drawn as non-indexed geometry and the PrimitiveType of the Mesh
         * determines how the vertices are interpreted when drawn.
         *
         * @param type The new primitive type.
         */
        void setPrimitiveType(Mesh::PrimitiveType type);

        /**
         * Sets the specified vertex data into the mapped vertex buffer.
         *
         * @param vertexData The vertex data to be set.
         * @param vertexStart The index of the starting vertex (0 by default).
         * @param vertexCount The number of vertices to be set (default is 0, for all vertices).
         */
        void setVertexData(const float* vertexData, size_t vertexStart = 0, size_t vertexCount = 0);

        void setRawVertexData(const float* vertexData, size_t vertexId, size_t numFloats);

        /**
         * Creates and adds a new part of primitive data defining how the vertices are connected.
         *
         * @param primitiveType The type of primitive data to connect the indices as.
         * @param indexFormat The format of the indices. SHORT or INT.
         * @param indexCount The number of indices to be contained in the part.
         * @param dynamic true if the index data is dynamic; false otherwise.
         *
         * @return The newly created/added mesh part.
         */
        std::shared_ptr<MeshPart> addPart(PrimitiveType primitiveType, Mesh::IndexFormat indexFormat, size_t indexCount, bool dynamic = false);

        /**
         * Gets the number of mesh parts contained within the mesh.
         *
         * @return The number of mesh parts contained within the mesh.
         */
        size_t getPartCount() const;

        /**
         * Gets a MeshPart by index.
         *
         * @param index The index of the MeshPart to get.
         *
         * @return The MeshPart at the specified index.
         */
        const std::shared_ptr<MeshPart>& getPart(size_t index);

        /**
         * Returns the bounding box for the points in this mesh.
         *
         * Only meshes loaded from bundle files are imported with valid
         * bounding volumes. Programmatically created meshes will contain
         * empty bounding volumes until the setBoundingBox and/or
         * setBoundingSphere methods are called to specify the mesh's
         * local bounds.
         *
         * Meshes that are attached to a Model with a MeshSkin will have
         * a bounding volume that is not necessarily tight fighting on the
         * Mesh vertices. Instead, the bounding volume will be an approximation
         * that contains all possible vertex positions in all possible poses after
         * skinning is applied. This is necessary since skinning vertices
         * result in vertex positions that lie outside the original mesh bounds
         * and could otherwise result in a bounding volume that does not fully
         * contain an animated/skinned mesh.
         *
         * @return The bounding box for the mesh.
         */
        const BoundingBox& getBoundingBox() const;

        /**
         * Sets the bounding box for this mesh.
         *
         * @param box The new bounding box for the mesh.
         */
        void setBoundingBox(const BoundingBox& box);

        /**
         * Destructor.
         */
        virtual ~Mesh();

    private:

        Mesh(const Mesh& copy) = delete;
        Mesh& operator=(const Mesh&) = delete;

        const VertexFormat _vertexFormat;
        size_t _vertexCount = 0;
        VertexBufferHandle _vertexBuffer{};
        PrimitiveType _primitiveType = TRIANGLES;
        std::vector<std::shared_ptr<MeshPart>> _parts{};
        bool _dynamic = false;
        BoundingBox _boundingBox;
    };
}
