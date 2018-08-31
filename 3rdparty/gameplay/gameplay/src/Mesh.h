#pragma once

#include "RenderState.h"
#include "Drawable.h"

#include "gl/structuredvertexbuffer.h"
#include "gsl_util.h"

#include <gsl/gsl>

#include <memory>

namespace gameplay
{
class MeshPart;


class Model;


class Mesh : public Drawable
{
public:
    explicit Mesh(const gl::StructuredVertexBuffer::AttributeMapping& mapping,
                  bool dynamic,
                  const std::string& label = {})
    {
        addBuffer( mapping, dynamic, label );
    }

    Mesh(const Mesh&) = delete;

    Mesh& operator=(const Mesh&) = delete;

    static gsl::not_null<std::shared_ptr<Mesh>> createQuadFullscreen(float width,
                                                                     float height,
                                                                     const gl::Program& program,
                                                                     bool invertY = false);

    void addPart(const gsl::not_null<std::shared_ptr<MeshPart>>& meshPart)
    {
        m_parts.emplace_back( meshPart );
    }

    std::size_t getPartCount() const
    {
        return m_parts.size();
    }

    const gsl::not_null<std::shared_ptr<MeshPart>>& getPart(std::size_t index)
    {
        BOOST_ASSERT( index < m_parts.size() );

        return m_parts[index];
    }

    ~Mesh() override = default;

    const gsl::not_null<std::shared_ptr<gl::StructuredVertexBuffer>>& getBuffer(std::size_t idx)
    {
        BOOST_ASSERT( idx < m_buffers.size() );

        return m_buffers[idx];
    }

    const std::vector<gsl::not_null<std::shared_ptr<gl::StructuredVertexBuffer>>>& getBuffers() const
    {
        return m_buffers;
    }

    std::vector<gsl::not_null<std::shared_ptr<gl::StructuredVertexBuffer>>>& getBuffers()
    {
        return m_buffers;
    }

    const std::vector<gsl::not_null<std::shared_ptr<MeshPart>>>& getParts() const noexcept
    {
        return m_parts;
    }

    void addBuffer(const gl::StructuredVertexBuffer::AttributeMapping& mapping,
                   bool dynamic,
                   const std::string& label = {})
    {
        m_buffers.emplace_back( make_not_null_shared<gl::StructuredVertexBuffer>( mapping, dynamic, label ) );
    }

    RenderState& getRenderState() override
    {
        return m_renderState;
    }

    virtual void draw(RenderContext& context);

private:
    RenderState m_renderState{};

    std::vector<gsl::not_null<std::shared_ptr<MeshPart>>> m_parts{};

    std::vector<gsl::not_null<std::shared_ptr<gl::StructuredVertexBuffer>>> m_buffers{};
};
}
