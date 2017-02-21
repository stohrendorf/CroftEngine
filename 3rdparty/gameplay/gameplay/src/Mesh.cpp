#include "Base.h"
#include "Mesh.h"
#include "MeshPart.h"
#include "Material.h"

#include <boost/log/trivial.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace gameplay
{
    std::shared_ptr<Mesh> Mesh::createQuad(float x, float y, float width, float height, float s1, float t1, float s2, float t2)
    {
        float x2 = x + width;
        float y2 = y + height;

        struct Vertex
        {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec2 uv;
        };

        const Vertex vertices[]{
            {{x, y2, 0.0f}, {0.0f, 0.0f, 1.0f}, {s1, t2}},
            {{x, y, 0.0f}, {0.0f, 0.0f, 1.0f}, {s1, t1}},
            {{x2, y2, 0.0f}, {0.0f, 0.0f, 1.0f}, {s2, t2}},
            {{x2, y, 0.0f}, {0.0f, 0.0f, 1.0f}, {s2, t1}}
        };

        ext::StructuredVertexBuffer::AttributeMapping attribs{
            { VERTEX_ATTRIBUTE_POSITION_NAME, ext::VertexAttribute{ GL_FLOAT, &Vertex::pos, 3 } },
            { VERTEX_ATTRIBUTE_NORMAL_NAME, ext::VertexAttribute{ GL_FLOAT, &Vertex::normal, 3 } },
            { VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, ext::VertexAttribute{ GL_FLOAT, &Vertex::uv, 2 } }
        };

        auto mesh = std::make_shared<Mesh>(attribs, false);
        mesh->getBuffer(0).assign<Vertex>(&vertices[0], 4);

        return mesh;
    }


    std::shared_ptr<Mesh> Mesh::createQuadFullscreen(float width, float height, bool invertY)
    {
        struct Vertex
        {
            glm::vec2 pos;
            glm::vec2 uv;
        };

        const Vertex vertices[]{
            {{0.0f, 0.0f},    {0.0f, invertY ? 0.0f : 1.0f}},
            {{width, 0.0f},   {1.0f, invertY ? 0.0f : 1.0f}},
            {{width, height}, {1.0f, invertY ? 1.0f : 0.0f}},
            {{0.0f, height},  {0.0f, invertY ? 1.0f : 0.0f}}
        };

        ext::StructuredVertexBuffer::AttributeMapping attribs{
            { VERTEX_ATTRIBUTE_POSITION_NAME, ext::VertexAttribute{ GL_FLOAT, &Vertex::pos, 2 } },
            { VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, ext::VertexAttribute{ GL_FLOAT, &Vertex::uv, 2 } }
        };

        auto mesh = std::make_shared<Mesh>(attribs, false);
        mesh->getBuffer(0).assign<Vertex>(vertices, 4);

        auto part = mesh->addPart(PrimitiveType::TRIANGLES, IndexFormat::INDEX16, 6, false);

        static const uint16_t indices[6] =
        {
            0, 1, 2,
            0, 2, 3
        };

        part->setIndexData(indices, 0, 6);

        return mesh;
    }


    std::shared_ptr<Mesh> Mesh::createQuad(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4)
    {
        // Calculate the normal vector of the plane.
        const glm::vec3 v1 = p2 - p1, v2 = p3 - p2;
        const auto n = glm::normalize(glm::cross(v1, v2));

        struct Vertex
        {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec2 uv;
        };

        const Vertex vertices[]{
            {p1, n, {0.0f, 1.0f}},
            {p2, n, {0.0f, 0.0f}},
            {p3, n, {1.0f, 1.0f}},
            {p4, n, {1.0f, 0.0f}}
        };

        ext::StructuredVertexBuffer::AttributeMapping attribs{
            { VERTEX_ATTRIBUTE_POSITION_NAME, ext::VertexAttribute{ GL_FLOAT, &Vertex::pos, 3 } },
            { VERTEX_ATTRIBUTE_NORMAL_NAME, ext::VertexAttribute{ GL_FLOAT, &Vertex::normal, 3 } },
            { VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, ext::VertexAttribute{ GL_FLOAT, &Vertex::uv, 2 } }
        };

        auto mesh = std::make_shared<Mesh>(attribs, false);
        mesh->getBuffer(0).assign<Vertex>(vertices, 4);

        return mesh;
    }


    std::shared_ptr<Mesh> Mesh::createLines(const gsl::not_null<const glm::vec3*>& points, size_t pointCount)
    {
        BOOST_ASSERT(pointCount > 0);

        ext::StructuredVertexBuffer::AttributeMapping attribs{
            { VERTEX_ATTRIBUTE_POSITION_NAME, ext::VertexAttribute{ GL_FLOAT, &glm::vec3::x, 3 } }
        };

        auto mesh = std::make_shared<Mesh>(attribs, false);
        mesh->getBuffer(0).assign(points, pointCount);

        return mesh;
    }


    std::shared_ptr<Mesh> Mesh::createBoundingBox(const BoundingBox& box)
    {
        glm::vec3 corners[8];
        box.getCorners(corners);

        glm::vec3 vertices[]{
            corners[7],
            corners[6],
            corners[1],
            corners[0],
            corners[7],
            corners[4],
            corners[3],
            corners[0],
            corners[0],
            corners[1],
            corners[2],
            corners[3],
            corners[4],
            corners[5],
            corners[2],
            corners[1],
            corners[6],
            corners[5]
        };

        ext::StructuredVertexBuffer::AttributeMapping attribs{
            { VERTEX_ATTRIBUTE_POSITION_NAME, ext::VertexAttribute{ GL_FLOAT, &glm::vec3::x, 3 } }
        };
        auto mesh = std::make_shared<Mesh>(attribs, false);
        mesh->getBuffer(0).assign<glm::vec3>(vertices, 18);

        return mesh;
    }


    std::shared_ptr<MeshPart> Mesh::addPart(PrimitiveType primitiveType, IndexFormat indexFormat, size_t indexCount, bool dynamic)
    {
        auto part = std::make_shared<MeshPart>(this, primitiveType, indexFormat, indexCount, dynamic);
        _parts.emplace_back(part);
        return part;
    }


    size_t Mesh::getPartCount() const
    {
        return _parts.size();
    }


    const std::shared_ptr<MeshPart>& Mesh::getPart(size_t index)
    {
        BOOST_ASSERT(index < _parts.size());

        return _parts[index];
    }
}
