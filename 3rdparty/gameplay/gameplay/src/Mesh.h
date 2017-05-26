#pragma once

#include "BoundingBox.h"

#include "ext/structuredvertexbuffer.h"

#include <gsl/gsl>

#include <memory>


namespace gameplay
{
    class MeshPart;
    class Model;


    /**
     * Defines a mesh supporting various vertex formats and 1 or more
     * MeshPart(s) to define how the vertices are connected.
     */
    class Mesh
    {
        friend class Model;

    public:
        explicit Mesh(const ext::StructuredVertexBuffer::AttributeMapping& mapping, bool dynamic, const std::string& label = {})
        {
            addBuffer(mapping, dynamic, label);
        }


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
        static std::shared_ptr<Mesh> createQuadFullscreen(float width, float height, bool invertY = false);

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
        static std::shared_ptr<Mesh> createLines(const gsl::not_null<const glm::vec3*>& points, size_t pointCount);

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
         * Creates and adds a new part of primitive data defining how the vertices are connected.
         *
         * @param primitiveType The type of primitive data to connect the indices as.
         * @param indexFormat The format of the indices. SHORT or INT.
         * @param indexCount The number of indices to be contained in the part.
         * @param dynamic true if the index data is dynamic; false otherwise.
         *
         * @return The newly created/added mesh part.
         */
        std::shared_ptr<MeshPart> addPart(GLenum primitiveType, GLint indexFormat, size_t indexCount, bool dynamic = false);

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
         * Destructor.
         */
        virtual ~Mesh() = default;


        ext::StructuredVertexBuffer& getBuffer(size_t idx)
        {
            BOOST_ASSERT(idx < m_buffers.size());

            return m_buffers[idx];
        }


        const std::vector<ext::StructuredVertexBuffer>& getBuffers() const
        {
            return m_buffers;
        }


        std::vector<ext::StructuredVertexBuffer>& getBuffers()
        {
            return m_buffers;
        }


        size_t addBuffer(const ext::StructuredVertexBuffer::AttributeMapping& mapping, bool dynamic, const std::string& label = {})
        {
            m_buffers.emplace_back(mapping, dynamic, label);
            return m_buffers.size() - 1;
        }

    private:

        Mesh(const Mesh& copy) = delete;
        Mesh& operator=(const Mesh&) = delete;

        std::vector<std::shared_ptr<MeshPart>> _parts{};
        std::vector<ext::StructuredVertexBuffer> m_buffers{};
    };
}
