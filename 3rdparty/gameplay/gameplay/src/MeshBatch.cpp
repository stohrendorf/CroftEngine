#include "Base.h"
#include "MeshBatch.h"
#include "Material.h"
#include "MeshPart.h"
#include "Model.h"

#include <boost/log/trivial.hpp>


namespace gameplay
{
    MeshBatch::MeshBatch(const VertexFormat& vertexFormat, Mesh::PrimitiveType primitiveType, const std::shared_ptr<Material>& material)
        : m_vertexFormat(vertexFormat)
        , m_primitiveType(primitiveType)
        , m_material(material)
        , m_vertices()
        , m_indices()
        , m_started(false)
    {
        BOOST_ASSERT(material);
    }


    MeshBatch::~MeshBatch() = default;


    void MeshBatch::add(const void* vertices, size_t vertexCount, const std::vector<uint16_t>& indices)
    {
        BOOST_ASSERT(vertices);

        // Copy index data.
        BOOST_ASSERT(!indices.empty());

        if( m_indices.empty() )
        {
            // Simply copy values directly into the start of the index array.
            m_indices = indices;
        }
        else
        {
            const auto currentVertexCount = m_vertices.size() / m_vertexFormat.getVertexSize();

            if( m_primitiveType == Mesh::TRIANGLE_STRIP )
            {
                // Create a degenerate triangle to connect separate triangle strips
                // by duplicating the previous and next vertices.
                m_indices.emplace_back(m_indices.back());
                m_indices.emplace_back(currentVertexCount);
            }

            // Loop through all indices and insert them, with their values offset by
            // 'vertexCount' so that they are relative to the first newly inserted vertex.
            for( size_t i = 0; i < indices.size(); ++i )
            {
                m_indices.emplace_back(indices[i] + currentVertexCount);
            }
        }

        // Copy vertex data.
        const auto vBytes = vertexCount * m_vertexFormat.getVertexSize();
        std::copy_n(static_cast<const uint8_t*>(vertices), vBytes, std::back_inserter(m_vertices));
    }


    void MeshBatch::start()
    {
        m_vertices.clear();
        m_indices.clear();
        m_started = true;
    }


    bool MeshBatch::isStarted() const
    {
        return m_started;
    }


    void MeshBatch::finish()
    {
        m_started = false;
    }


    void MeshBatch::draw()
    {
        BOOST_ASSERT(m_vertices.size() % m_vertexFormat.getVertexSize() == 0);

        if( m_vertices.empty() || m_indices.empty() )
            return; // nothing to draw

        BOOST_ASSERT(m_material);

        //! @todo Find a better way than creating a new model on each draw call.
        auto mesh = Mesh::createMesh(m_vertexFormat, 0, true);
        mesh->rebuild(reinterpret_cast<const float*>(m_vertices.data()), m_vertices.size() / m_vertexFormat.getVertexSize());
        auto part = mesh->addPart(m_primitiveType, Mesh::INDEX16, m_indices.size(), true);
        part->setMaterial(m_material);
        part->setIndexData(m_indices.data(), 0, m_indices.size());

        Model mdl{ mesh };
        mdl.draw();

        //pass->unbind();
    }
}
