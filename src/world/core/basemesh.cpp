#include "basemesh.h"

#include "engine/engine.h"
#include "render/shader_description.h"
#include "world/animation/texture.h"

#include <irrlicht.h>

namespace world
{
namespace core
{
void BaseMesh::polySortInMesh(const world::World& world)
{
    for(Polygon& p : m_polygons)
    {
        if(p.textureAnimationId && *p.textureAnimationId < world.m_textureAnimations.size())
        {
            const animation::TextureAnimationSequence* seq = &world.m_textureAnimations[*p.textureAnimationId];
            // set tex coordinates to the first frame for correct texture transform in renderer
            world.m_textureAtlas->getCoordinates(seq->textureIndices[0], false, p, 0, seq->uvrotate);
        }

        if(p.blendMode != loader::BlendingMode::Solid && p.blendMode != loader::BlendingMode::AlphaTransparency)
        {
            m_transparencyPolygons.emplace_back(p);
        }
    }
}

BaseMesh::~BaseMesh() = default;

void BaseMesh::genVBO(irr::scene::SMesh* mesh)
{
    BOOST_ASSERT(mesh != nullptr);
    if(mesh->getMeshBufferCount() > 0)
        return;
    
    mesh->addMeshBuffer(m_vertices);
    m_vertices->drop();
    mesh->addMeshBuffer(m_animatedVertices);
    m_animatedVertices->drop();
    
    // Store additional skinning information
    if(!m_matrixIndices.empty())
    {
        glGenBuffers(1, &m_vboSkinArray);
        glBindBuffer(GL_ARRAY_BUFFER, m_vboSkinArray);
        glBufferData(GL_ARRAY_BUFFER, m_matrixIndices.size() * sizeof(MatrixIndex), m_matrixIndices.data(), GL_STATIC_DRAW);
    }

    GLsizeiptr elementsSize = sizeof(GLuint) * m_alphaElements;
    for(size_t i = 0; i < m_texturePageCount; i++)
    {
        elementsSize += sizeof(GLuint) * m_elementsPerTexture[i];
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementsSize, m_elements.data(), GL_STATIC_DRAW);

    // Update references for transparent polygons
    for(render::TransparentPolygonReference& p : m_transparentPolygons)
    {
        p.used_vertex_array = p.isAnimated ? m_animatedVertexArray : m_mainVertexArray;
    }
}

irr::video::S3DVertex* BaseMesh::findVertex(const irr::core::vector3df& v)
{
    for(irr::video::S3DVertex& mv : m_vertices)
    {
        if(v.getDistanceFrom(mv.Pos) < 2.0)
        {
            return &mv;
        }
    }

    return nullptr;
}

/*
* FACES FUNCTIONS
*/
irr::s32 BaseMesh::findOrAddVertex(const irr::video::S3DVertex& vertex)
{
    auto ind = m_vertices->Vertices.binary_search(vertex);
    if(ind >=0)
        return ind;

    m_vertices->Vertices.push_back(vertex);

    return m_vertices.size() - 1;
}

irr::s32 BaseMesh::addAnimatedVertex(const irr::video::S3DVertex& vertex)
{
    // Skip search for equal vertex; tex coords may differ but aren't stored in
    // animated_vertex_s

    m_animatedVertices->Vertices.push_back(vertex);

    return m_animatedVertices->Vertices.size() - 1;
}

void BaseMesh::genFaces()
{
    m_elementsPerTexture.resize(m_texturePageCount);

    /*
    * Layout of the buffers:
    *
    * Normal vertex buffer:
    * - vertices of polygons in order, skipping only animated.
    * Animated vertex buffer:
    * - vertices (without tex coords) of polygons in order, skipping only
    *   non-animated.
    * Animated texture buffer:
    * - tex coords of polygons in order, skipping only non-animated.
    *   stream, initially empty.
    *
    * Normal elements:
    * - elements for texture[0]
    * ...
    * - elements for texture[n]
    * - elements for alpha
    * Animated elements:
    * - animated elements (opaque)
    * - animated elements (blended)
    */

    // Do a first pass to find the numbers of everything
    m_alphaElements = 0;
    size_t numNormalElements = 0;
    m_animatedElementCount = 0;
    m_alphaAnimatedElementCount = 0;

    size_t transparent = 0;
    for(const Polygon& p : m_polygons)
    {
        if(p.isBroken())
            continue;

        size_t elementCount = (p.vertices.size() - 2) * 3;
        if(p.isDoubleSided)
            elementCount *= 2;

        if(!p.textureAnimationId)
        {
            if(p.blendMode == loader::BlendingMode::Solid || p.blendMode == loader::BlendingMode::AlphaTransparency)
            {
                m_elementsPerTexture[p.textureIndex] += elementCount;
                numNormalElements += elementCount;
            }
            else
            {
                m_alphaElements += elementCount;
                ++transparent;
            }
        }
        else
        {
            if(p.blendMode == loader::BlendingMode::Solid || p.blendMode == loader::BlendingMode::AlphaTransparency)
                m_animatedElementCount += elementCount;
            else
            {
                m_alphaAnimatedElementCount += elementCount;
                ++transparent;
            }
        }
    }

    size_t elementOffset = 0;
    std::vector<size_t> startPerTexture(m_texturePageCount, 0);
    for(size_t i = 0; i < m_texturePageCount; i++)
    {
        startPerTexture[i] = elementOffset;
        elementOffset += m_elementsPerTexture[i];
    }
    size_t startTransparent = elementOffset;

    m_allAnimatedElements.resize(m_animatedElementCount + m_alphaAnimatedElementCount);
    size_t animatedStart = 0;
    size_t animatedStartTransparent = m_animatedElementCount;

    m_transparentPolygons.resize(transparent);
    size_t transparentPolygonStart = 0;

    for(const Polygon& p : m_polygons)
    {
        if(p.isBroken())
            continue;

        size_t elementCount = (p.vertices.size() - 2) * 3;
        if(p.isDoubleSided)
        {
            elementCount *= 2;
        }

        if(!p.textureAnimationId)
        {
            // Not animated

            size_t oldStart;
            if(p.blendMode == loader::BlendingMode::Solid || p.blendMode == loader::BlendingMode::AlphaTransparency)
            {
                oldStart = startPerTexture[p.textureIndex];
                startPerTexture[p.textureIndex] += elementCount;
            }
            else
            {
                oldStart = startTransparent;
                startTransparent += elementCount;
                m_transparentPolygons[transparentPolygonStart].firstIndex = oldStart;
                m_transparentPolygons[transparentPolygonStart].count = elementCount;
                m_transparentPolygons[transparentPolygonStart].polygon = &p;
                m_transparentPolygons[transparentPolygonStart].isAnimated = false;
                transparentPolygonStart += 1;
            }

            // Render the polygon as a triangle fan. That is obviously correct for
            // a triangle and also correct for any quad.
            auto firstVertex = findOrAddVertex(p.vertices[0]);
            auto previousVertex = findOrAddVertex(p.vertices[1]);
            
            for(size_t j = 2; j < p.vertices.size(); j++)
            {
                auto thisVertex = findOrAddVertex(p.vertices[j]);

                m_vertices->Indices.push_back(firstVertex);
                m_vertices->Indices.push_back(previousVertex);
                m_vertices->Indices.push_back(thisVertex);

                if(p.isDoubleSided)
                {
                    m_vertices->Indices.push_back(firstVertex);
                    m_vertices->Indices.push_back(thisVertex);
                    m_vertices->Indices.push_back(previousVertex);
                }

                previousVertex = thisVertex;
            }
        }
        else
        {
            // Animated
            size_t oldStart;
            if(p.blendMode == loader::BlendingMode::Solid || p.blendMode == loader::BlendingMode::AlphaTransparency)
            {
                oldStart = animatedStart;
                animatedStart += elementCount;
            }
            else
            {
                oldStart = animatedStartTransparent;
                animatedStartTransparent += elementCount;
                m_transparentPolygons[transparentPolygonStart].firstIndex = oldStart;
                m_transparentPolygons[transparentPolygonStart].count = elementCount;
                m_transparentPolygons[transparentPolygonStart].polygon = &p;
                m_transparentPolygons[transparentPolygonStart].isAnimated = true;
                transparentPolygonStart += 1;
            }

            // Render the polygon as a triangle fan. That is obviously correct for
            // a triangle and also correct for any quad.
            auto startVertex = addAnimatedVertex(p.vertices[0]);
            auto previousVertex = addAnimatedVertex(p.vertices[1]);

            for(size_t j = 2; j < p.vertices.size(); j++)
            {
                auto thisVertex = addAnimatedVertex(p.vertices[j]);

                m_animatedVertices->Indices.push_back(startVertex);
                m_animatedVertices->Indices.push_back(previousVertex);
                m_animatedVertices->Indices.push_back(thisVertex);

                if(p.isDoubleSided)
                {
                    m_animatedVertices->Indices.push_back(startVertex);
                    m_animatedVertices->Indices.push_back(thisVertex);
                    m_animatedVertices->Indices.push_back(previousVertex);
                }

                previousVertex = thisVertex;
            }
        }
    }
}

btCollisionShape *BT_CSfromMesh(const std::shared_ptr<BaseMesh>& mesh, bool useCompression, bool buildBvh, bool is_static)
{
    uint32_t cnt = 0;
    btTriangleMesh *trimesh = new btTriangleMesh;
    btCollisionShape* ret;

    for(const Polygon &p : mesh->m_polygons)
    {
        if(p.isBroken())
        {
            continue;
        }

        for(size_t j = 1; j + 1 < p.vertices.size(); j++)
        {
            const auto& v0 = p.vertices[j + 1].position;
            const auto& v1 = p.vertices[j].position;
            const auto& v2 = p.vertices[0].position;
            trimesh->addTriangle(util::convert(v0), util::convert(v1), util::convert(v2), true);
        }
        cnt++;
    }

    if(cnt == 0)
    {
        delete trimesh;
        return nullptr;
    }

    if(is_static)
    {
        ret = new btBvhTriangleMeshShape(trimesh, useCompression, buildBvh);
    }
    else
    {
        ret = new btConvexTriangleMeshShape(trimesh, true);
    }

    ret->setMargin(COLLISION_MARGIN_RIGIDBODY);

    return ret;
}
} // namespace core
} // namespace world
