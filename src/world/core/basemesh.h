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
    size_t m_alphaElements;

    irr::scene::SMeshBuffer* m_vertices = new irr::scene::SMeshBuffer();
    irr::scene::SMeshBuffer* m_animatedVertices = new irr::scene::SMeshBuffer();

    size_t m_animatedElementCount;
    size_t m_alphaAnimatedElementCount;

    std::vector<render::TransparentPolygonReference> m_transparentPolygons;

    irr::core::aabbox3df m_boundingBox;
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

    ~BaseMesh();

    void genVBO(irr::scene::SMesh* mesh);
    void genFaces();
    irr::s32 findOrAddVertex(const irr::video::S3DVertex& v);
    irr::s32 addAnimatedVertex(const irr::video::S3DVertex& v);
    void polySortInMesh(const world::World& world);
    irr::video::S3DVertex* findVertex(const irr::core::vector3df& v);
};

btCollisionShape* BT_CSfromMesh(const std::shared_ptr<BaseMesh> &mesh, bool useCompression, bool buildBvh, bool is_static = true);
} // namespace core
} // namespace world
