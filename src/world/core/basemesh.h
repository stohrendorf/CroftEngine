#pragma once

#include "boundingbox.h"
#include "polygon.h"
#include "render/render.h"
#include "world/animation/animation.h"

#include <vector>
#include <irrlicht.h>

namespace render
{
class VertexArray;
} // namespace render

namespace world
{
namespace core
{
class BaseMesh
{
public:
    bool m_usesVertexColors; //!< does this mesh have prebaked vertex lighting

    std::vector<Polygon> m_polygons;

    std::vector<Polygon> m_transparencyPolygons;

    size_t m_texturePageCount;
    std::vector<size_t> m_elementsPerTexture;
    std::vector<GLuint> m_elements;
    size_t m_alphaElements;

    std::vector<irr::video::S3DVertex> m_vertices;

    size_t m_animatedElementCount;
    size_t m_alphaAnimatedElementCount;
    std::vector<GLuint> m_allAnimatedElements;
    std::vector<animation::AnimatedVertex> m_animatedVertices;

    std::vector<render::TransparentPolygonReference> m_transparentPolygons;

    irr::core::vector3df m_center; //!< geometry center of mesh
    BoundingBox m_boundingBox; //!< AABB bounding volume
    irr::f32 m_radius; //!< radius of the bounding sphere

#pragma pack(push,1)
    struct MatrixIndex
    {
        int8_t i = 0, j = 0;

        explicit MatrixIndex() = default;

        MatrixIndex(int8_t i_, int8_t j_)
            : i(i_)
            , j(j_)
        {
        }
    };
#pragma pack(pop)

    std::vector<MatrixIndex> m_matrixIndices; //!< vertices map for skin mesh

    GLuint                m_vboVertexArray = 0;
    GLuint                m_vboIndexArray = 0;
    GLuint                m_vboSkinArray = 0;
    std::shared_ptr< render::VertexArray > m_mainVertexArray;

    // Buffers for animated polygons
    // The first contains position, normal and color.
    // The second contains the texture coordinates. It gets updated every frame.
    GLuint                m_animatedVboVertexArray;
    GLuint                m_animatedVboTexCoordArray;
    GLuint                m_animatedVboIndexArray;
    std::shared_ptr< render::VertexArray > m_animatedVertexArray;

    ~BaseMesh();

    void updateBoundingBox();
    void genVBO();
    void genFaces();
    size_t addVertex(const irr::video::S3DVertex& v);
    size_t addAnimatedVertex(const irr::video::S3DVertex& v);
    void polySortInMesh(const world::World& world);
    irr::video::S3DVertex* findVertex(const irr::core::vector3df& v);
};

btCollisionShape* BT_CSfromMesh(const std::shared_ptr<BaseMesh> &mesh, bool useCompression, bool buildBvh, bool is_static = true);
} // namespace core
} // namespace world
