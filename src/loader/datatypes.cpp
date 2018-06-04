#include "datatypes.h"

#include "level/level.h"
#include "render/textureanimator.h"
#include "util/vmath.h"

#include <glm/gtc/type_ptr.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>

namespace loader
{
namespace
{
#pragma pack(push, 1)


struct RenderVertex
{
    glm::vec3 position;
    glm::vec4 color;
    glm::vec3 normal{0.0f};

    static const gameplay::gl::StructuredVertexBuffer::AttributeMapping& getFormat()
    {
        static const gameplay::gl::StructuredVertexBuffer::AttributeMapping attribs{
                {VERTEX_ATTRIBUTE_POSITION_NAME, gameplay::gl::VertexAttribute{&RenderVertex::position}},
                {VERTEX_ATTRIBUTE_NORMAL_NAME,   gameplay::gl::VertexAttribute{&RenderVertex::normal}},
                {VERTEX_ATTRIBUTE_COLOR_NAME,    gameplay::gl::VertexAttribute{&RenderVertex::color}}
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
    std::shared_ptr<gameplay::Material> material;
};


struct RenderModel
{
    std::vector<MeshPart> m_parts;

    std::shared_ptr<gameplay::Model> toModel(const gsl::not_null<std::shared_ptr<gameplay::Mesh>>& mesh)
    {
        for( const MeshPart& localPart : m_parts )
        {
#ifndef NDEBUG
            for( auto idx : localPart.indices )
            {
                BOOST_ASSERT( idx < mesh->getBuffer( 0 )->getVertexCount() );
            }
#endif
            gameplay::gl::VertexArrayBuilder builder;

            auto indexBuffer = make_not_null_shared<gameplay::gl::IndexBuffer>();
            indexBuffer->setData( localPart.indices, true );
            builder.attach( indexBuffer );

            builder.attach( mesh->getBuffers() );

            auto part = make_not_null_shared<gameplay::MeshPart>(
                    builder.build( localPart.material->getShaderProgram()->getHandle() ) );
            part->setMaterial( localPart.material );
            mesh->addPart( part );
        }

        auto model = std::make_shared<gameplay::Model>();
        model->addMesh( mesh );

        return model;
    }
};


struct SpriteVertex
{
    glm::vec3 pos;

    glm::vec2 uv;

    glm::vec3 color{1.0f};
};

gsl::not_null<std::shared_ptr<gameplay::Mesh>>
createSpriteMesh(const loader::Sprite& sprite, const std::shared_ptr<gameplay::Material>& material)
{
    const SpriteVertex vertices[]{
            {{sprite.left_side,  sprite.top_side,    0}, {sprite.t0.x, sprite.t1.y}},
            {{sprite.right_side, sprite.top_side,    0}, {sprite.t1.x, sprite.t1.y}},
            {{sprite.right_side, sprite.bottom_side, 0}, {sprite.t1.x, sprite.t0.y}},
            {{sprite.left_side,  sprite.bottom_side, 0}, {sprite.t0.x, sprite.t0.y}}
    };

    gameplay::gl::StructuredVertexBuffer::AttributeMapping attribs{
            {VERTEX_ATTRIBUTE_POSITION_NAME,        gameplay::gl::VertexAttribute{&SpriteVertex::pos}},
            {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, gameplay::gl::VertexAttribute{&SpriteVertex::uv}},
            {VERTEX_ATTRIBUTE_COLOR_NAME,           gameplay::gl::VertexAttribute{&SpriteVertex::color}}
    };

    auto mesh = make_not_null_shared<gameplay::Mesh>( attribs, false );
    mesh->getBuffer( 0 )->assign<SpriteVertex>( to_not_null( &vertices[0] ), 4 );

    static const uint16_t indices[6] =
            {
                    0, 1, 2,
                    0, 2, 3
            };

    gameplay::gl::VertexArrayBuilder builder;

    auto indexBuffer = make_not_null_shared<gameplay::gl::IndexBuffer>();
    indexBuffer->setData( to_not_null( &indices[0] ), 6, false );
    builder.attach( indexBuffer );
    builder.attach( mesh->getBuffers() );

    auto part = make_not_null_shared<gameplay::MeshPart>( builder.build( material->getShaderProgram()->getHandle() ) );
    part->setMaterial( material );
    mesh->addPart( part );

    return mesh;
}
}

std::shared_ptr<gameplay::Node> Room::createSceneNode(
        size_t roomId,
        const level::Level& level,
        const std::vector<gsl::not_null<std::shared_ptr<gameplay::gl::Texture>>>& textures,
        const std::map<loader::TextureLayoutProxy::TextureKey, gsl::not_null<std::shared_ptr<gameplay::Material>>>& materials,
        const std::map<loader::TextureLayoutProxy::TextureKey, gsl::not_null<std::shared_ptr<gameplay::Material>>>& waterMaterials,
        const std::vector<gsl::not_null<std::shared_ptr<gameplay::Model>>>& staticMeshes,
        render::TextureAnimator& animator)
{
    auto spriteMaterial = std::make_shared<gameplay::Material>( "shaders/textured_2.vert",
                                                                "shaders/textured_2.frag" );
    spriteMaterial->setCullFace( false );

    spriteMaterial->getParameter( "u_modelViewMatrix" )
                  ->bind( [](const gameplay::Node& node, gameplay::gl::Program::ActiveUniform& uniform) {
                      auto m = node.getModelViewMatrix();
                      // clear out rotation component
                      for( int i : {0, 2} )
                          for( int j = 0; j < 3; ++j )
                              m[i][j] = i == j ? 1 : 0;

                      uniform.set( m );
                  } );

    spriteMaterial->getParameter( "u_modelMatrix" )->bindModelMatrix();
    spriteMaterial->getParameter( "u_projectionMatrix" )->bindProjectionMatrix();

    spriteMaterial->getParameter( "u_baseLight" )->set( 1.0f );
    spriteMaterial->getParameter( "u_baseLightDiff" )->set( 0.0f );
    spriteMaterial->getParameter( "u_lightPosition" )->set( glm::vec3{std::numeric_limits<float>::quiet_NaN()} );

    RenderModel renderModel;
    std::map<TextureLayoutProxy::TextureKey, size_t> texBuffers;
    std::vector<RenderVertex> vbuf;
    std::vector<glm::vec2> uvCoords;
    auto mesh = make_not_null_shared<gameplay::Mesh>( RenderVertex::getFormat(), false,
                                                      "Room:" + std::to_string( roomId ) );

    for( const QuadFace& quad : rectangles )
    {
        const TextureLayoutProxy& proxy = level.m_textureProxies.at( quad.proxyId );

        if( texBuffers.find( proxy.textureKey ) == texBuffers.end() )
        {
            texBuffers[proxy.textureKey] = renderModel.m_parts.size();
            renderModel.m_parts.emplace_back();
            auto it = isWaterRoom() ? waterMaterials.find( proxy.textureKey ) : materials.find( proxy.textureKey );
            Expects( it != (isWaterRoom() ? waterMaterials.end() : materials.end()) );
            renderModel.m_parts.back().material = it->second;
        }
        const auto partId = texBuffers[proxy.textureKey];

        const auto firstVertex = vbuf.size();
        for( int i = 0; i < 4; ++i )
        {
            RenderVertex iv;
            iv.position = vertices[quad.vertices[i]].position.toRenderSystem();
            iv.color = vertices[quad.vertices[i]].color;
            uvCoords.push_back( proxy.uvCoordinates[i].toGl() );
            vbuf.push_back( iv );
        }

        animator.registerVertex( quad.proxyId, mesh, 0, firstVertex + 0 );
        renderModel.m_parts[partId].indices
                                   .emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( firstVertex + 0 ) );
        animator.registerVertex( quad.proxyId, mesh, 1, firstVertex + 1 );
        renderModel.m_parts[partId].indices
                                   .emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( firstVertex + 1 ) );
        animator.registerVertex( quad.proxyId, mesh, 2, firstVertex + 2 );
        renderModel.m_parts[partId].indices
                                   .emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( firstVertex + 2 ) );
        animator.registerVertex( quad.proxyId, mesh, 0, firstVertex + 0 );
        renderModel.m_parts[partId].indices
                                   .emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( firstVertex + 0 ) );
        animator.registerVertex( quad.proxyId, mesh, 2, firstVertex + 2 );
        renderModel.m_parts[partId].indices
                                   .emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( firstVertex + 2 ) );
        animator.registerVertex( quad.proxyId, mesh, 3, firstVertex + 3 );
        renderModel.m_parts[partId].indices
                                   .emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( firstVertex + 3 ) );
    }
    for( const Triangle& tri : triangles )
    {
        const TextureLayoutProxy& proxy = level.m_textureProxies.at( tri.proxyId );

        if( texBuffers.find( proxy.textureKey ) == texBuffers.end() )
        {
            texBuffers[proxy.textureKey] = renderModel.m_parts.size();
            renderModel.m_parts.emplace_back();
            auto it = isWaterRoom() ? waterMaterials.find( proxy.textureKey ) : materials.find( proxy.textureKey );
            Expects( it != (isWaterRoom() ? waterMaterials.end() : materials.end()) );
            renderModel.m_parts.back().material = it->second;
        }
        const auto partId = texBuffers[proxy.textureKey];

        const auto firstVertex = vbuf.size();
        for( int i = 0; i < 3; ++i )
        {
            RenderVertex iv;
            iv.position = vertices[tri.vertices[i]].position.toRenderSystem();
            iv.color = vertices[tri.vertices[i]].color;
            uvCoords.push_back( proxy.uvCoordinates[i].toGl() );
            vbuf.push_back( iv );
        }

        animator.registerVertex( tri.proxyId, mesh, 0, firstVertex + 0 );
        renderModel.m_parts[partId].indices
                                   .emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( firstVertex + 0 ) );
        animator.registerVertex( tri.proxyId, mesh, 1, firstVertex + 1 );
        renderModel.m_parts[partId].indices
                                   .emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( firstVertex + 1 ) );
        animator.registerVertex( tri.proxyId, mesh, 2, firstVertex + 2 );
        renderModel.m_parts[partId].indices
                                   .emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( firstVertex + 2 ) );
    }

    mesh->getBuffer( 0 )->assign( vbuf );

    static const gameplay::gl::StructuredVertexBuffer::AttributeMapping attribs{
            {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, gameplay::gl::VertexAttribute{
                    gameplay::gl::VertexAttribute::SingleAttribute<glm::vec2>{}}}
    };

    mesh->addBuffer( attribs, true );
    mesh->getBuffer( 1 )->assign( uvCoords );

    auto resModel = renderModel.toModel( mesh );
    node = std::make_shared<gameplay::Node>( "Room:" + std::to_string( roomId ) );
    node->setDrawable( resModel );
    node->addMaterialParameterSetter( "u_lightPosition", [](const gameplay::Node& /*node*/,
                                                            gameplay::gl::Program::ActiveUniform& uniform) {
        uniform.set( glm::vec3{0.0f} );
    } );
    node->addMaterialParameterSetter( "u_baseLight", [](const gameplay::Node& /*node*/,
                                                        gameplay::gl::Program::ActiveUniform& uniform) {
        uniform.set( 1.0f );
    } );
    node->addMaterialParameterSetter( "u_baseLightDiff", [](const gameplay::Node& /*node*/,
                                                            gameplay::gl::Program::ActiveUniform& uniform) {
        uniform.set( 1.0f );
    } );

    for( const RoomStaticMesh& sm : this->staticMeshes )
    {
        auto idx = level.findStaticMeshIndexById( sm.meshId );
        BOOST_ASSERT( idx >= 0 );
        BOOST_ASSERT( static_cast<size_t>(idx) < staticMeshes.size() );
        auto subNode = std::make_shared<gameplay::Node>( "staticMesh" );
        subNode->setDrawable( staticMeshes[idx].get() );
        subNode->setLocalMatrix( glm::translate( glm::mat4{1.0f}, (sm.position - position).toRenderSystem() )
                                 * glm::rotate( glm::mat4{1.0f}, util::auToRad( sm.rotation ), glm::vec3{0, -1, 0} ) );

        subNode->addMaterialParameterSetter( "u_baseLight",
                                             [brightness = sm.getBrightness()](const gameplay::Node& /*node*/,
                                                                               gameplay::gl::Program::ActiveUniform& uniform) {
                                                 uniform.set( brightness );
                                             } );
        subNode->addMaterialParameterSetter( "u_baseLightDiff", [](const gameplay::Node& /*node*/,
                                                                   gameplay::gl::Program::ActiveUniform& uniform) {
            uniform.set( 0.0f );
        } );
        subNode->addMaterialParameterSetter( "u_lightPosition", [](const gameplay::Node& /*node*/,
                                                                   gameplay::gl::Program::ActiveUniform& uniform) {
            uniform.set( glm::vec3{std::numeric_limits<float>::quiet_NaN()} );
        } );
        node->addChild( subNode );
    }
    node->setLocalMatrix( glm::translate( glm::mat4{1.0f}, position.toRenderSystem() ) );

    for( const SpriteInstance& spriteInstance : sprites )
    {
        BOOST_ASSERT( spriteInstance.vertex < vertices.size() );
        BOOST_ASSERT( spriteInstance.id < level.m_sprites.size() );

        const Sprite& sprite = level.m_sprites[spriteInstance.id];
        BOOST_ASSERT( sprite.texture < textures.size() );

        auto spriteMesh = createSpriteMesh( sprite, spriteMaterial );
        auto model = std::make_shared<gameplay::Model>();
        model->addMesh( spriteMesh );

        auto spriteNode = std::make_shared<gameplay::Node>( "sprite" );
        spriteNode->setDrawable( model );
        const RoomVertex& v = vertices[spriteInstance.vertex];
        spriteNode->setLocalMatrix( glm::translate( glm::mat4{1.0f}, v.position.toRenderSystem() ) );
        spriteNode->addMaterialParameterSetter( "u_diffuseTexture",
                                                [texture = textures[sprite.texture]](const gameplay::Node& /*node*/,
                                                                                     gameplay::gl::Program::ActiveUniform& uniform) {
                                                    uniform.set( *texture );
                                                } );
        spriteNode->addMaterialParameterSetter( "u_baseLight",
                                                [brightness = v.getBrightness()](const gameplay::Node& /*node*/,
                                                                                 gameplay::gl::Program::ActiveUniform& uniform) {
                                                    uniform.set( brightness );
                                                } );

        node->addChild( spriteNode );
    }

    return node;
}

core::BoundingBox StaticMesh::getCollisionBox(const core::TRCoordinates& pos, core::Angle angle) const
{
    auto result = collision_box;

    const auto axis = core::axisFromAngle( angle, 45_deg );
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

void Room::patchHeightsForBlock(const engine::items::ItemNode& ctrl, int height)
{
    core::RoomBoundPosition pos = ctrl.m_state.position;
    //! @todo Ugly const_cast
    auto groundSector = const_cast<loader::Sector*>(ctrl.getLevel().findRealFloorSector( pos ).get());
    pos.position.Y += height - loader::SectorSize;
    const auto topSector = ctrl.getLevel().findRealFloorSector( pos );

    const auto q = height / loader::QuarterSectorSize;
    if( groundSector->floorHeight == -127 )
    {
        groundSector->floorHeight = topSector->ceilingHeight + q;
    }
    else
    {
        groundSector->floorHeight += q;
        if( groundSector->floorHeight == topSector->ceilingHeight )
            groundSector->floorHeight = -127;
    }

    if( groundSector->box == nullptr )
        return;

    if( (groundSector->box->overlap_index & 0x8000) == 0 )
        return;

    if( height >= 0 )
        groundSector->box->overlap_index &= ~0x4000;
    else
        groundSector->box->overlap_index |= 0x4000;
}
}
