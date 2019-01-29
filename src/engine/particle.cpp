#include "particle.h"

#include "level/level.h"
#include "engine/laranode.h"

namespace engine
{

void Particle::initDrawables(const level::Level& level)
{
    if( const auto& modelType = level.findAnimatedModelForType( object_number ) )
    {
        for( const auto& model : modelType->models )
        {
            m_drawables.emplace_back( model.get() );
        }
    }
    else if( const auto& spriteSequence = level.findSpriteSequenceForType( object_number ) )
    {
        shade = 4096;

        for( const loader::Sprite& spr : spriteSequence->sprites )
        {
            auto sprite = std::make_shared<gameplay::Sprite>( spr.x0, -spr.y0,
                                                              spr.x1, -spr.y1,
                                                              spr.t0, spr.t1,
                                                              level.m_spriteMaterial,
                                                              gameplay::Sprite::Axis::Y
            );
            m_drawables.emplace_back( sprite );
            m_spriteTextures.emplace_back( spr.texture );
        }

        addMaterialParameterSetter( "u_diffuseTexture", [this](const Node& /*node*/,
                                                               gameplay::gl::Program::ActiveUniform& uniform) {
            uniform.set( *m_spriteTextures.front() );
        } );
    }
    else
    {
        BOOST_LOG_TRIVIAL( warning ) << "Missing sprite/model referenced by particle: "
                                     << toString( object_number );
        return;
    }

    setDrawable( m_drawables.front() );
    m_lighting.bind( *this );
}

bool BloodSplatterParticle::update(level::Level& level)
{
    pos.position.X += speed * angle.Y.sin();
    pos.position.Z += speed * angle.Y.cos();
    ++timePerSpriteFrame;
    if( timePerSpriteFrame != 4 )
        return true;

    timePerSpriteFrame = 0;
    nextFrame();
    const auto& it = *level.m_spriteSequences.at( object_number );
    if( negSpriteFrameId <= it.length )
    {
        return false;
    }

    applyTransform();
    return true;
}

bool SplashParticle::update(level::Level& level)
{
    const auto& it = *level.m_spriteSequences.at( object_number );

    nextFrame();

    if( negSpriteFrameId <= it.length )
    {
        return false;
    }

    pos.position.X += speed * angle.Y.sin();
    pos.position.Z += speed * angle.Y.cos();

    applyTransform();
    return true;
}

bool BubbleParticle::update(level::Level& level)
{
    angle.X += 13_deg;
    angle.Y += 9_deg;
    pos.position.X += 11 * angle.Y.sin();
    pos.position.Y -= speed;
    pos.position.Z += 8 * angle.X.cos();
    auto sector = level::Level::findRealFloorSector( pos.position, &pos.room );
    if( sector == nullptr || !pos.room->isWaterRoom() )
    {
        return false;
    }

    const auto ceiling = HeightInfo::fromCeiling( sector, pos.position, level.m_itemNodes ).y;
    if( ceiling == -loader::HeightLimit || pos.position.Y <= ceiling )
    {
        return false;
    }

    applyTransform();
    return true;
}

bool FlameParticle::update(level::Level& level)
{
    nextFrame();
    if( negSpriteFrameId <= level.findSpriteSequenceForType( object_number )->length )
        negSpriteFrameId = 0;

    if( timePerSpriteFrame >= 0 )
    {
        level.playSound( TR1SoundId::Burning, pos.position.toRenderSystem() );
        if( timePerSpriteFrame != 0 )
        {
            --timePerSpriteFrame;
            applyTransform();
            return true;
        }

        if( level.m_lara->isNear( *this, 600 ) )
        {
            // it's hot here, isn't it?
            level.m_lara->m_state.health -= 3;
            level.m_lara->m_state.is_hit = true;

            const auto distSq = util::square( level.m_lara->m_state.position.position.X - pos.position.X )
                                + util::square( level.m_lara->m_state.position.position.Z - pos.position.Z );
            if( distSq < util::square( 300 ) )
            {
                timePerSpriteFrame = 100;

                const auto particle = std::make_shared<FlameParticle>( pos, level );
                particle->timePerSpriteFrame = -1;
                setParent( particle, pos.room->node );
                level.m_particles.emplace_back( particle );
            }
        }
    }
    else
    {
        // burn baby burn

        pos.position.X = 0;
        pos.position.Y = 0;
        if( timePerSpriteFrame == -1 )
        {
            pos.position.Z = -100;
        }
        else
        {
            pos.position.Z = 0;
        }

        const auto itemSpheres = level.m_lara->getSkeleton()->getBoneCollisionSpheres(
                level.m_lara->m_state,
                *level.m_lara->getSkeleton()->getInterpolationInfo( level.m_lara->m_state ).getNearestFrame(),
                nullptr );

        pos.position = core::TRVec{
                glm::vec3{translate( itemSpheres.at( -timePerSpriteFrame - 1 ).m,
                                     pos.position.toRenderSystem() )[3]}};

        const auto waterHeight = pos.room->getWaterSurfaceHeight( pos );
        if( !waterHeight.is_initialized() || waterHeight.get() >= pos.position.Y )
        {
            level.playSound( TR1SoundId::Burning, pos.position.toRenderSystem() );
            level.m_lara->m_state.health -= 3;
            level.m_lara->m_state.is_hit = true;
        }
        else
        {
            timePerSpriteFrame = 0;
            level.stopSound( TR1SoundId::Burning );
            return false;
        }
    }

    applyTransform();
    return true;
}
}
