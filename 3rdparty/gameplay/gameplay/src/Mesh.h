#pragma once

#include "gl/structuredvertexbuffer.h"

#include <gsl/gsl>

#include <memory>

namespace gameplay
{
class MeshPart;


class Model;


class Mesh
{
public:
    explicit Mesh(const gl::StructuredVertexBuffer::AttributeMapping& mapping,
                  bool dynamic,
                  const std::string& label = {})
    {
        addBuffer( mapping, dynamic, label );
    }

    static std::shared_ptr<Mesh> createQuadFullscreen(float width,
                                                      float height,
                                                      const gl::Program& program,
                                                      bool invertY = false);

    void addPart(const std::shared_ptr<MeshPart>& meshPart)
    {
        m_parts.emplace_back( meshPart );
    }

    std::size_t getPartCount() const
    {
        return m_parts.size();
    }

    const std::shared_ptr<MeshPart>& getPart(std::size_t index)
    {
        BOOST_ASSERT( index < m_parts.size() );

        return m_parts[index];
    }

    virtual ~Mesh() = default;

    const std::shared_ptr<gl::StructuredVertexBuffer>& getBuffer(std::size_t idx)
    {
        BOOST_ASSERT( idx < m_buffers.size() );

        return m_buffers[idx];
    }

    const std::vector<std::shared_ptr<gl::StructuredVertexBuffer>>& getBuffers() const
    {
        return m_buffers;
    }

    std::vector<std::shared_ptr<gl::StructuredVertexBuffer>>& getBuffers()
    {
        return m_buffers;
    }

    const std::vector<std::shared_ptr<MeshPart>>& getParts() const noexcept
    {
        return m_parts;
    }

    void addBuffer(const gl::StructuredVertexBuffer::AttributeMapping& mapping,
                   bool dynamic,
                   const std::string& label = {})
    {
        m_buffers.emplace_back( std::make_shared<gl::StructuredVertexBuffer>( mapping, dynamic, label ) );
    }

private:

    Mesh(const Mesh& copy) = delete;

    Mesh& operator=(const Mesh&) = delete;

    std::vector<std::shared_ptr<MeshPart>> m_parts{};

    std::vector<std::shared_ptr<gl::StructuredVertexBuffer>> m_buffers{};
};
}
