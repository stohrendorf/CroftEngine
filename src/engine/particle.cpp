#include "particle.h"

#include "level/level.h"

namespace engine
{

void FX::initDrawables(const level::Level& level)
{
    if( const auto& modelType = level.findAnimatedModelForType( object_number ) )
    {
        BOOST_ASSERT( modelType->frame_number + modelType->nmeshes <= level.m_sprites.size() );
        for( int i = 0; i < modelType->nmeshes; ++i )
        {
            auto model = level.m_models2[modelType->frame_number + i].get();
            m_drawables.emplace_back( model );
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

void BloodSplatterParticle::update(const level::Level& level)
{
    pos.position.X += speed * angle.Y.sin();
    pos.position.Z += speed * angle.Y.cos();
    ++timePerSpriteFrame;
    if( timePerSpriteFrame != 4 )
        return;

    timePerSpriteFrame = 0;
    nextFrame();
    auto it = level.m_spriteSequences.find( object_number );
    BOOST_ASSERT( it != level.m_spriteSequences.end() );
    if( negSpriteFrameId <= it->second->length )
    {
        // FIXME KillEffect( this );
    }

    applyTransform();
}

void SplashParticle::update(const level::Level& level)
{
    auto it = level.m_spriteSequences.find( object_number );
    BOOST_ASSERT( it != level.m_spriteSequences.end() );

    nextFrame();

    if( negSpriteFrameId <= it->second->length )
    {
        // FIXME KillEffect( this );
    }
    else
    {
        pos.position.X += speed * angle.Y.sin();
        pos.position.Z += speed * angle.Y.cos();
    }

    applyTransform();
}
}
