#pragma once

#include "RenderState.h"
#include "Drawable.h"

#include "gl/structuredvertexbuffer.h"

#include "gsl-lite.hpp"

#include <memory>

namespace gameplay
{
class MeshPart;


class Model;


class Mesh : public Drawable
{
public:
    explicit Mesh(const gl::StructuredVertexBuffer::AttributeMapping& mapping,
                  const bool dynamic,
                  const std::string& label = {})
    {
        addBuffer( mapping, dynamic, label );
    }

    Mesh(const Mesh&) = delete;

    Mesh(Mesh&&) = delete;

    Mesh& operator=(const Mesh&) = delete;

    Mesh& operator=(Mesh&&) = delete;

    static gsl::not_null<std::shared_ptr<Mesh>> createQuadFullscreen(float width,
                                                                     float height,
                                                                     const gl::Program& program,
                                                                     bool invertY = false);

    ~Mesh() override = default;

    void addBuffer(const gl::StructuredVertexBuffer::AttributeMapping& mapping,
                   bool dynamic,
                   const std::string& label = {})
    {
        m_buffers.emplace_back( std::make_shared<gl::StructuredVertexBuffer>( mapping, dynamic, label ) );
    }

    const std::vector<gsl::not_null<std::shared_ptr<gl::StructuredVertexBuffer>>>& getBuffers() const
    {
        return m_buffers;
    }

    void addPart(const gsl::not_null<std::shared_ptr<MeshPart>>& meshPart)
    {
        m_parts.emplace_back( meshPart );
    }

    const std::vector<gsl::not_null<std::shared_ptr<MeshPart>>>& getParts() const noexcept
    {
        return m_parts;
    }

    RenderState& getRenderState() override
    {
        return m_renderState;
    }

    void draw(RenderContext& context) override;

private:
    RenderState m_renderState{};

    std::vector<gsl::not_null<std::shared_ptr<MeshPart>>> m_parts{};

    std::vector<gsl::not_null<std::shared_ptr<gl::StructuredVertexBuffer>>> m_buffers{};
};
}
