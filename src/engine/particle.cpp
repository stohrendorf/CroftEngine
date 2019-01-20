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
        BOOST_ASSERT(
                gsl::narrow<size_t>( spriteSequence->offset - spriteSequence->length ) <= level.m_sprites.size() );

        shade = 4096;

        for( int i = 0; i > spriteSequence->length; --i )
        {
            const loader::Sprite& spr = level.m_sprites[spriteSequence->offset - i];

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

bool BloodSplatterParticle::update(const level::Level& level)
{
    pos.position.X += speed * angle.Y.sin();
    pos.position.Z += speed * angle.Y.cos();
    ++timePerSpriteFrame;
    if( timePerSpriteFrame != 4 )
        return true;

    timePerSpriteFrame = 0;
    nextFrame();
    const auto it = level.m_spriteSequences.find( object_number );
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
    const auto it = level.m_spriteSequences.find( object_number );
    BOOST_ASSERT( it != level.m_spriteSequences.end() );

    nextFrame();

    if( negSpriteFrameId <= it->second->length )
    {
        return false;
    }

    pos.position.X += speed * angle.Y.sin();
    pos.position.Z += speed * angle.Y.cos();

    applyTransform();
    return true;
}

bool BubbleParticle::update(const level::Level& level)
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
}
