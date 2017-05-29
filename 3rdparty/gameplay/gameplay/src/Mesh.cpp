#include "Base.h"
#include "Mesh.h"
#include "MeshPart.h"
#include "Material.h"

#include <boost/log/trivial.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace gameplay
{
    std::shared_ptr<Mesh> Mesh::createQuadFullscreen(float width, float height, bool invertY)
    {
        struct Vertex
        {
            glm::vec2 pos;

            glm::vec2 uv;
        };

        const Vertex vertices[]{
            {{0.0f, 0.0f}, {0.0f, invertY ? 0.0f : 1.0f}},
            {{width, 0.0f}, {1.0f, invertY ? 0.0f : 1.0f}},
            {{width, height}, {1.0f, invertY ? 1.0f : 0.0f}},
            {{0.0f, height}, {0.0f, invertY ? 1.0f : 0.0f}}
        };

        ext::StructuredVertexBuffer::AttributeMapping attribs{
            {VERTEX_ATTRIBUTE_POSITION_NAME, ext::VertexAttribute{&Vertex::pos}},
            {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, ext::VertexAttribute{&Vertex::uv}}
        };

        auto mesh = std::make_shared<Mesh>(attribs, false);
        mesh->getBuffer(0).assign<Vertex>(vertices, 4);

        static const uint16_t indices[6] =
        {
            0, 1, 2,
            0, 2, 3
        };

        auto part = mesh->addPart(gl::TypeTraits<decltype(indices[0])>::TypeId);

        part->setData(indices, 6, false);

        return mesh;
    }


    std::shared_ptr<MeshPart> Mesh::addPart(GLint indexFormat)
    {
        auto part = std::make_shared<MeshPart>(this, indexFormat);
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
