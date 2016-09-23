#include "Base.h"
#include "MeshBatch.h"
#include "Material.h"

#include <boost/log/trivial.hpp>

namespace gameplay
{

MeshBatch::MeshBatch(const VertexFormat& vertexFormat, Mesh::PrimitiveType primitiveType, const std::shared_ptr<Material>& material, bool indexed, unsigned int initialCapacity, unsigned int growSize)
    : m_vertexFormat(vertexFormat), m_primitiveType(primitiveType), m_material(material), m_indexed(indexed), m_capacity(0), m_growSize(growSize),
    m_vertexCapacity(0), m_indexCapacity(0), m_vertexCount(0), m_indexCount(0), m_vertices(nullptr), m_verticesPtr(nullptr), m_indices(nullptr), m_indicesPtr(nullptr), m_started(false)
{
    BOOST_ASSERT(material);
    resize(initialCapacity);
}

MeshBatch::~MeshBatch()
{
    SAFE_DELETE_ARRAY(m_vertices);
    SAFE_DELETE_ARRAY(m_indices);
}

void MeshBatch::add(const void* vertices, unsigned int vertexCount, const unsigned short* indices, unsigned int indexCount)
{
    BOOST_ASSERT(vertices);

    unsigned int newVertexCount = m_vertexCount + vertexCount;
    unsigned int newIndexCount = m_indexCount + indexCount;
    if (m_primitiveType == Mesh::TRIANGLE_STRIP && m_vertexCount > 0)
        newIndexCount += 2; // need an extra 2 indices for connecting strips with degenerate triangles

    // Do we need to grow the batch?
    while (newVertexCount > m_vertexCapacity || (m_indexed && newIndexCount > m_indexCapacity))
    {
        if (m_growSize == 0)
            return; // growing disabled, just clip batch
        if (!resize(m_capacity + m_growSize))
            return; // failed to grow
    }

    // Copy vertex data.
    BOOST_ASSERT(m_verticesPtr);
    unsigned int vBytes = vertexCount * m_vertexFormat.getVertexSize();
    memcpy(m_verticesPtr, vertices, vBytes);

    // Copy index data.
    if (m_indexed)
    {
        BOOST_ASSERT(indices);
        BOOST_ASSERT(m_indicesPtr);

        if (m_vertexCount == 0)
        {
            // Simply copy values directly into the start of the index array.
            memcpy(m_indicesPtr, indices, indexCount * sizeof(unsigned short));
        }
        else
        {
            if (m_primitiveType == Mesh::TRIANGLE_STRIP)
            {
                // Create a degenerate triangle to connect separate triangle strips
                // by duplicating the previous and next vertices.
                m_indicesPtr[0] = *(m_indicesPtr-1);
                m_indicesPtr[1] = m_vertexCount;
                m_indicesPtr += 2;
            }

            // Loop through all indices and insert them, with their values offset by
            // 'vertexCount' so that they are relative to the first newly inserted vertex.
            for (unsigned int i = 0; i < indexCount; ++i)
            {
                m_indicesPtr[i] = indices[i] + m_vertexCount;
            }
        }
        m_indicesPtr += indexCount;
        m_indexCount = newIndexCount;
    }

    m_verticesPtr += vBytes;
    m_vertexCount = newVertexCount;
}

void MeshBatch::updateVertexAttributeBinding()
{
    BOOST_ASSERT(m_material);

    // Update our vertex attribute bindings.
    auto t = m_material->getTechnique();
    BOOST_ASSERT(t);
    auto p = t->getPass();
    BOOST_ASSERT(p);
    _vaBinding = VertexAttributeBinding::create(m_vertexFormat, m_vertices, p->getShaderProgram());
}

unsigned int MeshBatch::getCapacity() const
{
    return m_capacity;
}

void MeshBatch::setCapacity(size_t capacity)
{
    resize(capacity);
}

bool MeshBatch::resize(size_t capacity)
{
    if (capacity == 0)
    {
        BOOST_LOG_TRIVIAL(error) << "Invalid resize capacity (0).";
        return false;
    }

    if (capacity == m_capacity)
        return true;

    // Store old batch data.
    unsigned char* oldVertices = m_vertices;
    unsigned short* oldIndices = m_indices;

    size_t vertexCapacity = 0;
    switch (m_primitiveType)
    {
    case Mesh::LINES:
        vertexCapacity = capacity * 2;
        break;
    case Mesh::LINE_STRIP:
        vertexCapacity = capacity + 1;
        break;
    case Mesh::POINTS:
        vertexCapacity = capacity;
        break;
    case Mesh::TRIANGLES:
        vertexCapacity = capacity * 3;
        break;
    case Mesh::TRIANGLE_STRIP:
        vertexCapacity = capacity + 2;
        break;
    default:
        BOOST_LOG_TRIVIAL(error) << "Unsupported primitive type for mesh batch (" << m_primitiveType << ").";
        return false;
    }

    // We have no way of knowing how many vertices will be stored in the batch
    // (we only know how many indices will be stored). Assume the worst case
    // for now, which is the same number of vertices as indices.
    size_t indexCapacity = vertexCapacity;
    if (m_indexed && indexCapacity > std::numeric_limits<unsigned short>::max())
    {
        BOOST_LOG_TRIVIAL(error) << "Index capacity is greater than the maximum unsigned short value (" << indexCapacity << " > " << std::numeric_limits<unsigned short>::max() << ").";
        return false;
    }

    // Allocate new data and reset pointers.
    size_t voffset = m_verticesPtr - m_vertices;
    size_t vBytes = vertexCapacity * m_vertexFormat.getVertexSize();
    m_vertices = new unsigned char[vBytes];
    if (voffset >= vBytes)
        voffset = vBytes - 1;
    m_verticesPtr = m_vertices + voffset;

    if (m_indexed)
    {
        size_t ioffset = m_indicesPtr - m_indices;
        m_indices = new unsigned short[indexCapacity];
        if (ioffset >= indexCapacity)
            ioffset = indexCapacity - 1;
        m_indicesPtr = m_indices + ioffset;
    }

    // Copy old data back in
    if (oldVertices)
        memcpy(m_vertices, oldVertices, std::min(m_vertexCapacity, vertexCapacity) * m_vertexFormat.getVertexSize());
    SAFE_DELETE_ARRAY(oldVertices);
    if (oldIndices)
        memcpy(m_indices, oldIndices, std::min(m_indexCapacity, indexCapacity) * sizeof(unsigned short));
    SAFE_DELETE_ARRAY(oldIndices);

    // Assign new capacities
    m_capacity = capacity;
    m_vertexCapacity = vertexCapacity;
    m_indexCapacity = indexCapacity;

    // Update our vertex attribute bindings now that our client array pointers have changed
    updateVertexAttributeBinding();

    return true;
}

void MeshBatch::start()
{
    m_vertexCount = 0;
    m_indexCount = 0;
    m_verticesPtr = m_vertices;
    m_indicesPtr = m_indices;
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
    if (m_vertexCount == 0 || (m_indexed && m_indexCount == 0))
        return; // nothing to draw

    // Not using VBOs, so unbind the element array buffer.
    // ARRAY_BUFFER will be unbound automatically during pass->bind().
    GL_ASSERT( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0 ) );

    BOOST_ASSERT(m_material);
    if (m_indexed)
        BOOST_ASSERT(m_indices);

    // Bind the material.
    auto technique = m_material->getTechnique();
    BOOST_ASSERT(technique);
    auto pass = technique->getPass();
    BOOST_ASSERT(pass);
    pass->bind(_vaBinding);

    if (m_indexed)
    {
        GL_ASSERT( glDrawElements(m_primitiveType, m_indexCount, GL_UNSIGNED_SHORT, static_cast<GLvoid*>(m_indices)) );
    }
    else
    {
        GL_ASSERT( glDrawArrays(m_primitiveType, 0, m_vertexCount) );
    }

    pass->unbind();
}


}
