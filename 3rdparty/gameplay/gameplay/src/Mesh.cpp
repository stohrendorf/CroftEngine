#include "Base.h"
#include "Mesh.h"
#include "MeshPart.h"
#include "Material.h"

#include <boost/log/trivial.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace gameplay
{
    Mesh::Mesh(const VertexFormat& vertexFormat, size_t vertexCount, bool dynamic)
        : _vertexFormat{vertexFormat}
        , _vertexCount{vertexCount}
        , _dynamic{dynamic}
    {
        GL_ASSERT(glGenBuffers(1, &_vertexBuffer));
        GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer));
        GL_ASSERT(glBufferData(GL_ARRAY_BUFFER, vertexFormat.getVertexSize() * vertexCount, nullptr, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
        GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }


    Mesh::~Mesh()
    {
        _parts.clear();

        if( _vertexBuffer )
        {
            glDeleteBuffers(1, &_vertexBuffer);
            _vertexBuffer = 0;
        }
    }


    void Mesh::rebuild(const float* vertexData, size_t vertexCount)
    {
        GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer));
        GL_ASSERT(glBufferData(GL_ARRAY_BUFFER, _vertexFormat.getVertexSize() * vertexCount, vertexData, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
        GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, 0));

        _vertexCount = vertexCount;
    }


    std::shared_ptr<Mesh> Mesh::createQuad(float x, float y, float width, float height, float s1, float t1, float s2, float t2)
    {
        float x2 = x + width;
        float y2 = y + height;

        float vertices[] =
        {
            x, y2, 0, 0, 0, 1, s1, t2,
            x, y, 0, 0, 0, 1, s1, t1,
            x2, y2, 0, 0, 0, 1, s2, t2,
            x2, y, 0, 0, 0, 1, s2, t1,
        };

        VertexFormat::Element elements[] =
        {
            VertexFormat::Element(VertexFormat::POSITION, 3),
            VertexFormat::Element(VertexFormat::NORMAL, 3),
            VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
        };
        auto mesh = std::make_shared<Mesh>(VertexFormat(elements, 3), 4, false);
        mesh->_primitiveType = TRIANGLE_STRIP;
        mesh->setVertexData(vertices, 0, 4);

        return mesh;
    }


    std::shared_ptr<Mesh> Mesh::createQuadFullscreen()
    {
        float x = -1.0f;
        float y = -1.0f;
        float x2 = 1.0f;
        float y2 = 1.0f;

        float vertices[] =
        {
            x, y2, 0, 1,
            x, y, 0, 0,
            x2, y2, 1, 1,
            x2, y, 1, 0
        };

        VertexFormat::Element elements[] =
        {
            VertexFormat::Element(VertexFormat::POSITION, 2),
            VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
        };
        auto mesh = std::make_shared<Mesh>(VertexFormat(elements, 2), 4, false);
        mesh->_primitiveType = TRIANGLE_STRIP;
        mesh->setVertexData(vertices, 0, 4);

        return mesh;
    }


    std::shared_ptr<Mesh> Mesh::createQuad(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4)
    {
        // Calculate the normal vector of the plane.
        glm::vec3 v1 = p2 - p1, v2 = p3 - p2;
        auto n = glm::normalize(glm::cross(v1, v2));

        float vertices[] =
        {
            p1.x, p1.y, p1.z, n.x, n.y, n.z, 0, 1,
            p2.x, p2.y, p2.z, n.x, n.y, n.z, 0, 0,
            p3.x, p3.y, p3.z, n.x, n.y, n.z, 1, 1,
            p4.x, p4.y, p4.z, n.x, n.y, n.z, 1, 0
        };

        VertexFormat::Element elements[] =
        {
            VertexFormat::Element(VertexFormat::POSITION, 3),
            VertexFormat::Element(VertexFormat::NORMAL, 3),
            VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
        };

        auto mesh = std::make_shared<Mesh>(VertexFormat(elements, 3), 4, false);

        mesh->_primitiveType = TRIANGLE_STRIP;
        mesh->setVertexData(vertices, 0, 4);

        return mesh;
    }


    std::shared_ptr<Mesh> Mesh::createLines(glm::vec3* points, size_t pointCount)
    {
        BOOST_ASSERT(points);
        BOOST_ASSERT(pointCount);

        VertexFormat::Element elements[] =
        {
            VertexFormat::Element(VertexFormat::POSITION, 3)
        };
        auto mesh = std::make_shared<Mesh>(VertexFormat(elements, 1), pointCount, false);

        mesh->_primitiveType = LINE_STRIP;
        mesh->setVertexData(glm::value_ptr(*points), 0, pointCount);

        return mesh;
    }


    std::shared_ptr<Mesh> Mesh::createBoundingBox(const BoundingBox& box)
    {
        glm::vec3 corners[8];
        box.getCorners(corners);

        float vertices[] =
        {
            corners[7].x, corners[7].y, corners[7].z,
            corners[6].x, corners[6].y, corners[6].z,
            corners[1].x, corners[1].y, corners[1].z,
            corners[0].x, corners[0].y, corners[0].z,
            corners[7].x, corners[7].y, corners[7].z,
            corners[4].x, corners[4].y, corners[4].z,
            corners[3].x, corners[3].y, corners[3].z,
            corners[0].x, corners[0].y, corners[0].z,
            corners[0].x, corners[0].y, corners[0].z,
            corners[1].x, corners[1].y, corners[1].z,
            corners[2].x, corners[2].y, corners[2].z,
            corners[3].x, corners[3].y, corners[3].z,
            corners[4].x, corners[4].y, corners[4].z,
            corners[5].x, corners[5].y, corners[5].z,
            corners[2].x, corners[2].y, corners[2].z,
            corners[1].x, corners[1].y, corners[1].z,
            corners[6].x, corners[6].y, corners[6].z,
            corners[5].x, corners[5].y, corners[5].z
        };

        VertexFormat::Element elements[] =
        {
            VertexFormat::Element(VertexFormat::POSITION, 3)
        };
        auto mesh = std::make_shared<Mesh>(VertexFormat(elements, 1), 18, false);
        mesh->_primitiveType = LINE_STRIP;
        mesh->setVertexData(vertices, 0, 18);

        return mesh;
    }


    const VertexFormat& Mesh::getVertexFormat() const
    {
        return _vertexFormat;
    }


    size_t Mesh::getVertexCount() const
    {
        return _vertexCount;
    }


    size_t Mesh::getVertexSize() const
    {
        return _vertexFormat.getVertexSize();
    }


    VertexBufferHandle Mesh::getVertexBuffer() const
    {
        return _vertexBuffer;
    }


    bool Mesh::isDynamic() const
    {
        return _dynamic;
    }


    Mesh::PrimitiveType Mesh::getPrimitiveType() const
    {
        return _primitiveType;
    }


    void Mesh::setPrimitiveType(PrimitiveType type)
    {
        _primitiveType = type;
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void Mesh::setVertexData(const float* vertexData, size_t vertexStart, size_t vertexCount)
    {
        GL_ASSERT( glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer) );

        if( vertexStart == 0 && vertexCount == 0 )
        {
            GL_ASSERT( glBufferData(GL_ARRAY_BUFFER, _vertexFormat.getVertexSize() * _vertexCount, vertexData, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW) );
        }
        else
        {
            if( vertexCount == 0 )
            {
                vertexCount = _vertexCount - vertexStart;
            }

            GL_ASSERT( glBufferSubData(GL_ARRAY_BUFFER, vertexStart * _vertexFormat.getVertexSize(), vertexCount * _vertexFormat.getVertexSize(), vertexData) );
        }
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void Mesh::setRawVertexData(const float* vertexData, size_t vertexId, size_t numFloats)
    {
        GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer));
        GL_ASSERT(glBufferSubData(GL_ARRAY_BUFFER, vertexId * _vertexFormat.getVertexSize(), numFloats * sizeof(float), vertexData));
    }


    std::shared_ptr<MeshPart> Mesh::addPart(PrimitiveType primitiveType, IndexFormat indexFormat, size_t indexCount, bool dynamic)
    {
        auto part = MeshPart::create(shared_from_this(), primitiveType, indexFormat, indexCount, dynamic);
        if( part )
        {
            _parts.emplace_back(part);
        }

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


    const BoundingBox& Mesh::getBoundingBox() const
    {
        return _boundingBox;
    }


    void Mesh::setBoundingBox(const BoundingBox& box)
    {
        _boundingBox = box;
    }
}
