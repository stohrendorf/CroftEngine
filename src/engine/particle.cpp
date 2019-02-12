#include "particle.h"

#include "loader/file/level/level.h"
#include "engine/laranode.h"

namespace engine
{

void Particle::initDrawables(const loader::file::level::Level& level, const float scale)
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

        for( const loader::file::Sprite& spr : spriteSequence->sprites )
        {
            auto sprite = std::make_shared<gameplay::Sprite>( spr.x0 * scale, -spr.y0 * scale,
                                                              spr.x1 * scale, -spr.y1 * scale,
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

glm::vec3 Particle::getPosition() const
{
    return pos.position.toRenderSystem();
}

Particle::Particle(const std::string& id,
                   const TR1ItemId objectNumber,
                   const gsl::not_null<const loader::file::Room*>& room,
                   loader::file::level::Level& level,
                   float scale)
        : Node{id}, Emitter{&level.m_soundEngine}, pos{room}, object_number{objectNumber}
{
    initDrawables( level, scale );
}

Particle::Particle(const std::string& id,
                   const TR1ItemId objectNumber,
                   const core::RoomBoundPosition& pos,
                   loader::file::level::Level& level,
                   float scale)
        : Node{id}, Emitter{&level.m_soundEngine}, pos{pos}, object_number{objectNumber}
{
    initDrawables( level, scale );
}

bool BloodSplatterParticle::update(loader::file::level::Level& level)
{
    pos.position += util::pitch( speed * 1_frame, angle.Y );
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

bool SplashParticle::update(loader::file::level::Level& level)
{
    const auto& it = *level.m_spriteSequences.at( object_number );

    nextFrame();

    if( negSpriteFrameId <= it.length )
    {
        return false;
    }

    pos.position += util::pitch( speed * 1_frame, angle.Y );

    applyTransform();
    return true;
}

bool BubbleParticle::update(loader::file::level::Level& level)
{
    angle.X += 13_deg;
    angle.Y += 9_deg;
    pos.position.X += util::sin( 11_len, angle.Y );
    pos.position.Y -= 1_frame * speed;
    pos.position.Z += util::cos( 8_len, angle.X );
    auto sector = loader::file::level::Level::findRealFloorSector( pos.position, &pos.room );
    if( sector == nullptr || !pos.room->isWaterRoom() )
    {
        return false;
    }

    const auto ceiling = HeightInfo::fromCeiling( sector, pos.position, level.m_itemNodes ).y;
    if( ceiling == -core::HeightLimit || pos.position.Y <= ceiling )
    {
        return false;
    }

    applyTransform();
    return true;
}

bool FlameParticle::update(loader::file::level::Level& level)
{
    nextFrame();
    if( negSpriteFrameId <= level.findSpriteSequenceForType( object_number )->length )
        negSpriteFrameId = 0;

    if( timePerSpriteFrame >= 0 )
    {
        level.playSound( TR1SoundId::Burning, this );
        if( timePerSpriteFrame != 0 )
        {
            --timePerSpriteFrame;
            applyTransform();
            return true;
        }

        if( level.m_lara->isNear( *this, 600_len ) )
        {
            // it's hot here, isn't it?
            level.m_lara->m_state.health -= 3_hp;
            level.m_lara->m_state.is_hit = true;

            const auto distSq = util::square( level.m_lara->m_state.position.position.X - pos.position.X )
                                + util::square( level.m_lara->m_state.position.position.Z - pos.position.Z );
            if( distSq < util::square( 300_len ) )
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

        pos.position = {0_len, 0_len, 0_len};
        if( timePerSpriteFrame == -1 )
        {
            pos.position.Y = -100_len;
        }
        else
        {
            pos.position.Y = 0_len;
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
            level.playSound( TR1SoundId::Burning, this );
            level.m_lara->m_state.health -= 3_hp;
            level.m_lara->m_state.is_hit = true;
        }
        else
        {
            timePerSpriteFrame = 0;
            level.stopSound( TR1SoundId::Burning, this );
            return false;
        }
    }

    applyTransform();
    return true;
}
}
