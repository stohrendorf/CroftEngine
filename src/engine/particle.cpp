#include "particle.h"

#include "level/level.h"

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
        BOOST_ASSERT( spriteSequence->offset - spriteSequence->length <= level.m_sprites.size() );

        shade = 4096;

        for( int i = 0; i > spriteSequence->length; --i )
        {
            const loader::Sprite& spr = level.m_sprites[spriteSequence->offset - i];

            auto sprite = std::make_shared<gameplay::Sprite>(
                    spr.left_side,
                    spr.bottom_side,
                    spr.right_side - spr.left_side,
                    spr.top_side - spr.bottom_side,
                    spr.t0,
                    spr.t1,
                    to_not_null( level.m_spriteMaterial ),
                    gameplay::Sprite::Axis::Y
            );
            m_drawables.emplace_back( sprite );
            m_spriteTextures.emplace_back( spr.texture );
        }
    }
    else
    {
        BOOST_LOG_TRIVIAL( warning ) << "Missing sprite/model referenced by particle: "
                                     << engine::toString( object_number );
        return;
    }

    setDrawable( m_drawables.front() );
    addMaterialParameterSetter( "u_diffuseTexture", [this](const gameplay::Node& /*node*/,
                                                           gameplay::gl::Program::ActiveUniform& uniform) {
        uniform.set( *m_spriteTextures.front() );
    } );
    m_lighting.bind( *this );
}

bool BloodSplatterParticle::update(const level::Level& level)
{
    pos.position.X += speed * angle.Y.sin();
    pos.position.Z += speed * angle.Y.cos();
    ++timePerSpriteFrame;
    if( timePerSpriteFrame != 4 )
        return true;

    timePerSpriteFrame = 0;
    nextFrame();
    auto it = level.m_spriteSequences.find( object_number );
    BOOST_ASSERT( it != level.m_spriteSequences.end() );

    if( negSpriteFrameId <= it->second->length )
    {
        return false;
    }

    applyTransform();
    return true;
}

bool SplashParticle::update(const level::Level& level)
{
    auto it = level.m_spriteSequences.find( object_number );
    BOOST_ASSERT( it != level.m_spriteSequences.end() );

    nextFrame();

    if( negSpriteFrameId <= it->second->length )
    {
        return false;
    }
    else
    {
        pos.position.X += speed * angle.Y.sin();
        pos.position.Z += speed * angle.Y.cos();
    }

    applyTransform();
    return true;
}

bool BubbleParticle::update(const level::Level& level)
{
    angle.X += 13_deg;
    angle.Y += 9_deg;
    core::TRCoordinates testPos{
            static_cast<int>(11 * angle.Y.sin() + pos.position.X),
            pos.position.Y - speed,
            static_cast<int>((8 * angle.X.cos()) + pos.position.Z)
    };
    auto sector = level.findRealFloorSector( testPos, to_not_null( &pos.room ) );
    if( sector == nullptr || !pos.room->isWaterRoom() )
    {
        return false;
    }

    auto ceiling = HeightInfo::fromCeiling( to_not_null( sector ), testPos, level.m_itemNodes ).y;
    if( ceiling == -loader::HeightLimit || testPos.Y <= ceiling )
    {
        return false;
    }

    pos.position = testPos;
    return true;
}
}
