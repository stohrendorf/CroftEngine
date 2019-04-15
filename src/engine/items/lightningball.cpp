#include "lightningball.h"

#include "engine/heightinfo.h"
#include "engine/laranode.h"
#include "render/scene/names.h"
#include "render/scene/mesh.h"
#include "render/gl/vertexarray.h"
#include "render/gl/indexbuffer.h"

namespace engine
{
namespace items
{
namespace
{
gsl::not_null<std::shared_ptr<render::scene::Mesh>>
createBolt(uint16_t points, const gsl::not_null<std::shared_ptr<render::scene::ShaderProgram>>& program, GLfloat lineWidth)
{
    std::vector<glm::vec3> vertices( points );

    static const render::gl::StructuredVertexBuffer::AttributeMapping attribs{
            {VERTEX_ATTRIBUTE_POSITION_NAME, render::gl::VertexAttribute{
                    render::gl::VertexAttribute::SingleAttribute<glm::vec3>{}}}
    };


    std::vector<uint16_t> indices;
    for( uint16_t i = 0; i < points; ++i )
        indices.emplace_back( i );

    auto indexBuffer = std::make_shared<render::gl::IndexBuffer>();
    indexBuffer->setData( indices, false );

    auto vb = std::make_shared<render::gl::StructuredVertexBuffer>(attribs, true);
    vb->assign<glm::vec3>( &vertices[0], points );

    auto vao = std::make_shared<render::gl::VertexArray>(indexBuffer, vb, program->getHandle());
    auto mesh = std::make_shared<render::scene::Mesh>( vao, GL_LINE_STRIP );

    mesh->getRenderState().setLineSmooth( true );
    mesh->getRenderState().setLineWidth( lineWidth );

    auto material = std::make_shared<render::scene::Material>( program );
    material->getParameter( "u_modelViewMatrix" )->bindModelViewMatrix();
    material->getParameter( "u_projectionMatrix" )->bindProjectionMatrix();

    mesh->setMaterial( material );

    return mesh;
}

using Bolt = std::array<core::TRVec, LightningBall::SegmentPoints>;

Bolt updateBolt(core::TRVec start, const core::TRVec& end, const render::scene::Mesh& mesh)
{
    const auto segmentSize = (end - start) / LightningBall::SegmentPoints;

    Bolt bolt;

    BOOST_ASSERT( mesh.getVAO()->getVertexBuffers()[0]->getVertexCount() == LightningBall::SegmentPoints );
    auto boltData = mesh.getVAO()->getVertexBuffers()[0]->mapTypedRw<glm::vec3>();
    for( size_t j = 0; j < LightningBall::SegmentPoints; j++ )
    {
        core::TRVec buckling{
                util::rand15s( core::QuarterSectorSize ),
                util::rand15s( core::QuarterSectorSize ),
                util::rand15s( core::QuarterSectorSize )
        };

        if( j == LightningBall::SegmentPoints - 1 )
            buckling.Y = 0_len;

        bolt[j] = start + buckling;
        boltData[j] = bolt[j].toRenderSystem();
        start += segmentSize;
    }
    mesh.getVAO()->getVertexBuffers()[0]->unmap();

    return bolt;
}
}

LightningBall::LightningBall(const gsl::not_null<Engine*>& engine,
                             const gsl::not_null<const loader::file::Room*>& room,
                             const loader::file::Item& item,
                             const loader::file::SkeletalModelType& animatedModel,
                             const gsl::not_null<std::shared_ptr<render::scene::ShaderProgram>>& boltProgram)
        : ModelItemNode{engine, room, item, true, animatedModel}
{
    if( animatedModel.nMeshes >= 1 )
    {
        m_poles = static_cast<size_t>(animatedModel.nMeshes - 1);
    }

    for( size_t i = 1; i < getSkeleton()->getChildren().size(); ++i )
    {
        getSkeleton()->getChildren()[i]->setDrawable( nullptr );
        getSkeleton()->getChildren()[i]->setVisible( false );
    }

    m_mainBoltMesh = createBolt( SegmentPoints, boltProgram, 10 );
    auto node = std::make_shared<render::scene::Node>( "lightning-bolt-main" );
    node->setDrawable( m_mainBoltMesh );
    addChild( getSkeleton(), node );

    for( auto& childBolt : m_childBolts )
    {
        childBolt.mesh = createBolt( SegmentPoints, boltProgram, 3 );

        node = std::make_shared<render::scene::Node>( "lightning-bolt-child" );
        node->setDrawable( childBolt.mesh );
        addChild( getSkeleton(), node );
    }
}

void LightningBall::update()
{
    if( !m_state.updateActivationTimeout() )
    {
        m_chargeTimeout = 1;
        m_shooting = false;
        m_laraHit = false;
        if( getEngine().roomsAreSwapped() )
            getEngine().swapAllRooms();

        deactivate();
        m_state.triggerState = TriggerState::Inactive;
        prepareRender();
        return;
    }

    prepareRender();

    if( --m_chargeTimeout > 0 )
        return;

    if( m_shooting != 0 )
    {
        m_shooting = false;
        m_chargeTimeout = 35 + util::rand15( 45 );
        m_laraHit = false;
        if( getEngine().roomsAreSwapped() )
            getEngine().swapAllRooms();

        return;
    }

    m_shooting = true;
    m_chargeTimeout = 20;
    m_laraHit = false;

    const auto radius = m_poles == 0 ? core::SectorSize : core::SectorSize * 5 / 2;
    if( getEngine().getLara().isNear( *this, radius ) )
    {
        // target at lara
        m_mainBoltEnd = getEngine().getLara().m_state.position.position - m_state.position.position;
        m_mainBoltEnd = core::TRVec{
                glm::vec3( (-m_state.rotation).toMatrix() * glm::vec4( m_mainBoltEnd.toRenderSystem(), 1.0f ) )};

        getEngine().getLara().m_state.health -= 400_hp;
        getEngine().getLara().m_state.is_hit = true;

        m_laraHit = true;
    }
    else if( m_poles == 0 )
    {
        // we don't have poles, so just shoot downwards
        m_mainBoltEnd = core::TRVec{};
        const auto sector = loader::file::findRealFloorSector( m_state.position );
        m_mainBoltEnd.Y = -HeightInfo::fromFloor( sector, m_state.position.position, getEngine().getItemNodes() ).y;
        m_mainBoltEnd.Y -= m_state.position.position.Y;
    }
    else
    {
        // select a random "pole"
        const auto itemSpheres = getSkeleton()->getBoneCollisionSpheres(
                m_state,
                *getSkeleton()->getInterpolationInfo( m_state ).getNearestFrame(),
                nullptr );
        m_mainBoltEnd = core::TRVec{itemSpheres[util::rand15( itemSpheres.size() - 1 ) + 1].getPosition()}
                        - m_state.position.position;
        m_mainBoltEnd = core::TRVec{
                glm::vec3( (-m_state.rotation).toMatrix() * glm::vec4( m_mainBoltEnd.toRenderSystem(), 1.0f ) )};
    }

    for( auto& childBolt : m_childBolts )
    {
        childBolt.startIndex = util::rand15( SegmentPoints - 1 );
        childBolt.end = m_mainBoltEnd + core::TRVec{
                util::rand15s( core::QuarterSectorSize ),
                0_len,
                util::rand15s( core::QuarterSectorSize )};
    }

    if( !getEngine().roomsAreSwapped() )
        getEngine().swapAllRooms();

    playSoundEffect( TR1SoundId::Chatter );
}

void LightningBall::collide(LaraNode& lara, CollisionInfo& info)
{
    if( !m_laraHit )
        return;

    lara.hit_direction = static_cast<core::Axis>(util::rand15( 4 ));
    lara.hit_frame += 1_frame;
    if( lara.hit_frame > 34_frame )
        lara.hit_frame = 34_frame;
}

void LightningBall::prepareRender()
{
    ModelItemNode::update();

    if( m_shooting == 0 )
    {
        for( size_t i = 1; i < getSkeleton()->getChildren().size(); ++i )
        {
            getSkeleton()->getChildren()[i]->setVisible( false );
        }
        return;
    }

    for( size_t i = 1; i < getSkeleton()->getChildren().size(); ++i )
    {
        getSkeleton()->getChildren()[i]->setVisible( i - 1 >= m_poles );
    }

    const auto nearestFrame = getSkeleton()->getInterpolationInfo( m_state ).getNearestFrame();
    const auto segmentStart = core::TRVec{glm::vec3(
            core::fromPackedAngles( nearestFrame->getAngleData()[0] ) * glm::vec4( nearestFrame->pos.toGl(), 1.0f ) )};

    const Bolt mainBolt = updateBolt( segmentStart, m_mainBoltEnd, *m_mainBoltMesh );

    for( const auto& childBolt : m_childBolts )
    {
        updateBolt( mainBolt[childBolt.startIndex], childBolt.end, *childBolt.mesh );
    }
}

void LightningBall::load(const YAML::Node& n)
{
    ModelItemNode::load( n );

    while( getSkeleton()->getChildren().size() > m_poles + 1 )
    {
        setParent( getSkeleton()->getChildren().back(), nullptr );
    }

    auto node = std::make_shared<render::scene::Node>( "lightning-bolt-main" );
    node->setDrawable( m_mainBoltMesh );
    addChild( getSkeleton(), node );
    for( auto& childBolt : m_childBolts )
    {
        node = std::make_shared<render::scene::Node>( "lightning-bolt-child" );
        node->setDrawable( childBolt.mesh );
        addChild( getSkeleton(), node );
    }
}
}
}
