#include "datatypes.h"

#include "engine/engine.h"
#include "render/textureanimator.h"
#include "util/helpers.h"
#include "level/level.h"
#include "render/scene/names.h"
#include "render/scene/mesh.h"
#include "render/scene/Material.h"
#include "render/scene/Sprite.h"
#include "render/gl/vertexarray.h"

#include <glm/gtc/type_ptr.hpp>

#include <boost/range/adaptors.hpp>

namespace loader
{
namespace file
{
namespace
{
#pragma pack(push, 1)


struct RenderVertex
{
    glm::vec3 position;
    glm::vec4 color;
    glm::vec3 normal{0.0f};

    static const render::gl::StructuredVertexBuffer::AttributeMapping& getFormat()
    {
        static const render::gl::StructuredVertexBuffer::AttributeMapping attribs{
                {VERTEX_ATTRIBUTE_POSITION_NAME, render::gl::VertexAttribute{&RenderVertex::position}},
                {VERTEX_ATTRIBUTE_NORMAL_NAME,   render::gl::VertexAttribute{&RenderVertex::normal}},
                {VERTEX_ATTRIBUTE_COLOR_NAME,    render::gl::VertexAttribute{&RenderVertex::color}}
        };

        return attribs;
    }
};


#pragma pack(pop)

struct MeshPart
{
    using IndexBuffer = std::vector<uint16_t>;
    static_assert( std::is_unsigned<IndexBuffer::value_type>::value, "Index buffer entries must be unsigned" );

    IndexBuffer indices;
    std::shared_ptr<render::scene::Material> material;
};


struct RenderModel
{
    std::vector<MeshPart> m_parts;

    std::shared_ptr<render::scene::Model> toModel(
            const gsl::not_null<std::shared_ptr<render::gl::StructuredVertexBuffer>>& vbuf,
            const gsl::not_null<std::shared_ptr<render::gl::StructuredVertexBuffer>>& uvBuf)
    {
        auto model = std::make_shared<render::scene::Model>();

        for( const MeshPart& localPart : m_parts )
        {
#ifndef NDEBUG
            for( auto idx : localPart.indices )
            {
                BOOST_ASSERT( idx < vbuf->getVertexCount() );
            }
#endif

            auto indexBuffer = std::make_shared<render::gl::IndexBuffer>();
            indexBuffer->setData( localPart.indices, true );

            std::vector<gsl::not_null<std::shared_ptr<render::gl::IndexBuffer>>> indexBufs{indexBuffer};
            std::vector<gsl::not_null<std::shared_ptr<render::gl::StructuredVertexBuffer>>> vBufs{vbuf, uvBuf};

            auto mesh = std::make_shared<render::scene::Mesh>(
                    std::make_shared<render::gl::VertexArray>( indexBufs,
                                                               vBufs,
                                                               localPart.material->getShaderProgram()->getHandle() )
            );
            mesh->setMaterial( localPart.material );
            model->addMesh( mesh );
        }

        return model;
    }
};
}

void Room::createSceneNode(
        const size_t roomId,
        const level::Level& level,
        const std::map<TextureKey, gsl::not_null<std::shared_ptr<render::scene::Material>>>& materials,
        const std::map<TextureKey, gsl::not_null<std::shared_ptr<render::scene::Material>>>& waterMaterials,
        const std::vector<gsl::not_null<std::shared_ptr<render::scene::Model>>>& staticMeshes,
        render::TextureAnimator& animator,
        const std::shared_ptr<render::scene::Material>& spriteMaterial,
        const std::shared_ptr<render::scene::Material>& portalMaterial
)
{
    RenderModel renderModel;
    std::map<TextureKey, size_t> texBuffers;
    std::vector<RenderVertex> vbufData;
    std::vector<glm::vec2> uvCoordsData;

    const auto label = "Room:" + std::to_string( roomId );
    auto vbuf = std::make_shared<render::gl::StructuredVertexBuffer>( RenderVertex::getFormat(), false, label );

    static const render::gl::StructuredVertexBuffer::AttributeMapping uvAttribs{
            {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, render::gl::VertexAttribute{
                    render::gl::VertexAttribute::SingleAttribute<glm::vec2>{}}}
    };
    auto uvCoords = std::make_shared<render::gl::StructuredVertexBuffer>( uvAttribs, true, label + "-uv" );

    for( const QuadFace& quad : rectangles )
    {
        const TextureLayoutProxy& proxy = level.m_textureProxies.at( quad.proxyId.get() );

        if( texBuffers.find( proxy.textureKey ) == texBuffers.end() )
        {
            texBuffers[proxy.textureKey] = renderModel.m_parts.size();

            renderModel.m_parts.emplace_back();

            auto it = isWaterRoom() ? waterMaterials.at( proxy.textureKey ) : materials.at( proxy.textureKey );
            renderModel.m_parts.back().material = it;
        }
        const auto partId = texBuffers[proxy.textureKey];

        const auto firstVertex = vbufData.size();
        for( int i = 0; i < 4; ++i )
        {
            RenderVertex iv;
            iv.position = quad.vertices[i].from( vertices ).position.toRenderSystem();
            iv.color = quad.vertices[i].from( vertices ).color;
            uvCoordsData.push_back( proxy.uvCoordinates[i].toGl() );

            // generate normal
            if( i <= 2 )
            {
                static const int indices[3] = {0, 1, 2};
                const auto o = quad.vertices[indices[(i + 0) % 3]].from( vertices ).position.toRenderSystem();
                const auto a = quad.vertices[indices[(i + 1) % 3]].from( vertices ).position.toRenderSystem();
                const auto b = quad.vertices[indices[(i + 2) % 3]].from( vertices ).position.toRenderSystem();
                iv.normal = glm::normalize( -glm::cross( a - o, b - o ) );
            }
            else
            {
                static const int indices[3] = {0, 2, 3};
                const auto o = quad.vertices[indices[(i + 0) % 3]].from( vertices ).position.toRenderSystem();
                const auto a = quad.vertices[indices[(i + 1) % 3]].from( vertices ).position.toRenderSystem();
                const auto b = quad.vertices[indices[(i + 2) % 3]].from( vertices ).position.toRenderSystem();
                iv.normal = glm::normalize( -glm::cross( a - o, b - o ) );
            }

            vbufData.push_back( iv );
        }

        for( int i : {0, 1, 2, 0, 2, 3} )
        {
            animator.registerVertex( quad.proxyId, uvCoords, i, firstVertex + i );
            renderModel.m_parts[partId].indices.emplace_back(
                    gsl::narrow<MeshPart::IndexBuffer::value_type>( firstVertex + i )
            );
        }
    }
    for( const Triangle& tri : triangles )
    {
        const TextureLayoutProxy& proxy = level.m_textureProxies.at( tri.proxyId.get() );

        if( texBuffers.find( proxy.textureKey ) == texBuffers.end() )
        {
            texBuffers[proxy.textureKey] = renderModel.m_parts.size();

            renderModel.m_parts.emplace_back();

            auto it = isWaterRoom() ? waterMaterials.at( proxy.textureKey ) : materials.at( proxy.textureKey );
            renderModel.m_parts.back().material = it;
        }
        const auto partId = texBuffers[proxy.textureKey];

        const auto firstVertex = vbufData.size();
        for( int i = 0; i < 3; ++i )
        {
            RenderVertex iv;
            iv.position = tri.vertices[i].from( vertices ).position.toRenderSystem();
            iv.color = tri.vertices[i].from( vertices ).color;
            uvCoordsData.push_back( proxy.uvCoordinates[i].toGl() );

            static const int indices[3] = {0, 1, 2};
            const auto o = tri.vertices[indices[(i + 0) % 3]].from( vertices ).position.toRenderSystem();
            const auto a = tri.vertices[indices[(i + 1) % 3]].from( vertices ).position.toRenderSystem();
            const auto b = tri.vertices[indices[(i + 2) % 3]].from( vertices ).position.toRenderSystem();
            iv.normal = glm::normalize( -glm::cross( a - o, b - o ) );

            vbufData.push_back( iv );
        }

        for( int i:{0, 1, 2} )
        {
            animator.registerVertex( tri.proxyId, uvCoords, i, firstVertex + i );
            renderModel.m_parts[partId].indices
                                       .emplace_back(
                                               gsl::narrow<MeshPart::IndexBuffer::value_type>( firstVertex + i ) );
        }
    }

    vbuf->assign( vbufData );

    uvCoords->assign( uvCoordsData );

    auto resModel = renderModel.toModel( vbuf, uvCoords );
    resModel->getRenderState().setCullFace( true );
    resModel->getRenderState().setCullFaceSide( GL_BACK );

    node = std::make_shared<render::scene::Node>( "Room:" + std::to_string( roomId ) );
    node->setDrawable( resModel );
    node->addMaterialParameterSetter( "u_lightAmbient", [](const render::scene::Node& /*node*/,
                                                           render::gl::Program::ActiveUniform& uniform) {
        uniform.set( 1.0f );
    } );
    node->addMaterialParameterSetter( "u_numLights", [](const render::scene::Node& /*node*/,
                                                        render::gl::Program::ActiveUniform& uniform) {
        uniform.set( 0 );
    } );

    for( const RoomStaticMesh& sm : this->staticMeshes )
    {
        const auto idx = level.findStaticMeshIndexById( sm.meshId );
        if( idx < 0 )
            continue;

        auto subNode = std::make_shared<render::scene::Node>( "staticMesh" );
        subNode->setDrawable( staticMeshes.at( idx ).get() );
        subNode->setLocalMatrix( translate( glm::mat4{1.0f}, (sm.position - position).toRenderSystem() )
                                 * rotate( glm::mat4{1.0f}, toRad( sm.rotation ), glm::vec3{0, -1, 0} ) );

        subNode->addMaterialParameterSetter( "u_lightAmbient",
                                             [brightness = sm.getBrightness()](const render::scene::Node& /*node*/,
                                                                               render::gl::Program::ActiveUniform& uniform) {
                                                 uniform.set( brightness );
                                             } );
        subNode->addMaterialParameterSetter( "u_numLights", [](const render::scene::Node& /*node*/,
                                                               render::gl::Program::ActiveUniform& uniform
        ) {
            uniform.set( 0 );
        } );
        addChild( node, subNode );
    }
    node->setLocalMatrix( translate( glm::mat4{1.0f}, position.toRenderSystem() ) );

    for( const SpriteInstance& spriteInstance : sprites )
    {
        BOOST_ASSERT( spriteInstance.vertex.get() < vertices.size() );

        const Sprite& sprite = level.m_sprites.at( spriteInstance.id.get() );

        const auto model = std::make_shared<render::scene::Sprite>( sprite.x0, -sprite.y0,
                                                                    sprite.x1, -sprite.y1,
                                                                    sprite.t0, sprite.t1,
                                                                    spriteMaterial,
                                                                    render::scene::Sprite::Axis::Y );

        auto spriteNode = std::make_shared<render::scene::Node>( "sprite" );
        spriteNode->setDrawable( model );
        const RoomVertex& v = vertices.at( spriteInstance.vertex.get() );
        spriteNode->setLocalMatrix( translate( glm::mat4{1.0f}, v.position.toRenderSystem() ) );
        spriteNode->addMaterialParameterSetter( "u_diffuseTexture",
                                                [texture = sprite.texture](const render::scene::Node& /*node*/,
                                                                           render::gl::Program::ActiveUniform& uniform
                                                ) { uniform.set( *texture ); } );
        spriteNode->addMaterialParameterSetter( "u_lightAmbient",
                                                [brightness = v.getBrightness()](const render::scene::Node& /*node*/,
                                                                                 render::gl::Program::ActiveUniform& uniform
                                                ) { uniform.set( brightness ); } );

        addChild( node, spriteNode );
    }
    for( auto& portal : portals )
        portal.buildMesh( portalMaterial );
}

core::BoundingBox StaticMesh::getCollisionBox(const core::TRVec& pos, const core::Angle angle) const
{
    auto result = collision_box;

    const auto axis = axisFromAngle( angle, 45_deg );
    switch( *axis )
    {
        case core::Axis::PosZ:
            // nothing to do
            break;
        case core::Axis::PosX:
            result.min.X = collision_box.min.Z;
            result.max.X = collision_box.max.Z;
            result.min.Z = -collision_box.max.X;
            result.max.Z = -collision_box.min.X;
            break;
        case core::Axis::NegZ:
            result.min.X = -collision_box.max.X;
            result.max.X = -collision_box.min.X;
            result.min.Z = -collision_box.max.Z;
            result.max.Z = -collision_box.min.Z;
            break;
        case core::Axis::NegX:
            result.min.X = -collision_box.max.Z;
            result.max.X = -collision_box.min.Z;
            result.min.Z = collision_box.min.X;
            result.max.Z = collision_box.max.X;
            break;
    }

    result.min += pos;
    result.max += pos;
    return result;
}

void Room::patchHeightsForBlock(const engine::items::ItemNode& item, const core::Length height)
{
    auto room = item.m_state.position.room;
    //! @todo Ugly const_cast
    auto groundSector = const_cast<Sector*>(loader::file::findRealFloorSector( item.m_state.position.position, &room ));
    Expects( groundSector != nullptr );
    const auto topSector = loader::file::findRealFloorSector(
            item.m_state.position.position + core::TRVec{0_len, height - core::SectorSize, 0_len}, &room );

    if( groundSector->floorHeight == -core::HeightLimit )
    {
        groundSector->floorHeight = topSector->ceilingHeight + height;
    }
    else
    {
        groundSector->floorHeight = topSector->floorHeight + height;
        if( groundSector->floorHeight == topSector->ceilingHeight )
            groundSector->floorHeight = -core::HeightLimit;
    }

    Expects( groundSector->box != nullptr );

    if( !groundSector->box->isBlockable() )
        return;

    if( height >= 0_len )
        groundSector->box->unblock();
    else
        groundSector->box->block();
}

const Sector* findRealFloorSector(const core::TRVec& position, const gsl::not_null<gsl::not_null<const Room*>*>& room)
{
    const Sector* sector;
    // follow portals
    while( true )
    {
        sector = (*room)->findFloorSectorWithClampedIndex( (position.X - (*room)->position.X) / core::SectorSize,
                                                           (position.Z - (*room)->position.Z) / core::SectorSize );
        if( sector->portalTarget == nullptr )
        {
            break;
        }

        *room = sector->portalTarget;
    }

    // go up/down until we are in the room that contains our coordinates
    Expects( sector != nullptr );
    if( position.Y >= sector->floorHeight )
    {
        while( position.Y >= sector->floorHeight && sector->roomBelow != nullptr )
        {
            *room = sector->roomBelow;
            sector = (*room)->getSectorByAbsolutePosition( position );
            if( sector == nullptr )
                return nullptr;
        }
    }
    else
    {
        while( position.Y < sector->ceilingHeight && sector->roomAbove != nullptr )
        {
            *room = sector->roomAbove;
            sector = (*room)->getSectorByAbsolutePosition( position );
            if( sector == nullptr )
                return nullptr;
        }
    }

    return sector;
}
}
}
