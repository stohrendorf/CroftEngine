#include "mesh.h"

#include "material.h"
#include "names.h"

#include <utility>

namespace render
{
namespace scene
{
gsl::not_null<std::shared_ptr<Mesh>>
    createQuadFullscreen(const float width, const float height, const gl::Program& program, const bool invertY)
{
    struct Vertex
    {
        glm::vec2 pos;

        glm::vec2 uv;
    };

    const Vertex vertices[]{{{0.0f, 0.0f}, {0.0f, invertY ? 0.0f : 1.0f}},
                            {{width, 0.0f}, {1.0f, invertY ? 0.0f : 1.0f}},
                            {{width, height}, {1.0f, invertY ? 1.0f : 0.0f}},
                            {{0.0f, height}, {0.0f, invertY ? 1.0f : 0.0f}}};

    static const gl::StructureLayout<Vertex> attribs{{VERTEX_ATTRIBUTE_POSITION_NAME, &Vertex::pos},
                                                            {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &Vertex::uv}};

    auto vertexBuffer = std::make_shared<gl::StructuredArrayBuffer<Vertex>>(attribs);
    vertexBuffer->setData(&vertices[0], 4, ::gl::BufferUsageARB::StaticDraw);

    static const uint16_t indices[6] = {0, 1, 2, 0, 2, 3};

    auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<uint16_t>>();
    indexBuffer->setData(&indices[0], 6, ::gl::BufferUsageARB::StaticDraw);

    return std::make_shared<MeshImpl<uint16_t, Vertex>>(
        std::make_shared<gl::VertexArray<uint16_t, Vertex>>(indexBuffer, vertexBuffer, program));
}

Mesh::~Mesh() = default;

void Mesh::render(RenderContext& context)
{
    if(m_material == nullptr)
        return;

    BOOST_ASSERT(context.getCurrentNode() != nullptr);

    context.pushState(getRenderState());

    for(const auto& setter : m_materialUniformSetters)
        setter(*context.getCurrentNode(), *m_material);

    context.pushState(m_material->getRenderState());
    context.bindState();

    m_material->bind(*context.getCurrentNode());

    drawIndexBuffers(m_primitiveType);

    context.popState();
    context.popState();
}
} // namespace scene
} // namespace render
