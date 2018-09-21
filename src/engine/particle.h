#pragma once

#include "core/angle.h"
#include "core/vec.h"
#include "items_tr1.h"
#include "Node.h"
#include "level/level.h"
#include "engine/lighting.h"

#include <memory>
#include <deque>

namespace engine
{
class Particle : public gameplay::Node
{
public:
    core::RoomBoundPosition pos;
    core::TRRotation angle;
    const engine::TR1ItemId object_number;
    int16_t speed = 0;
    int16_t fall_speed = 0;
    int16_t negSpriteFrameId = 0;
    int16_t timePerSpriteFrame = 0;
    int16_t shade = 4096;

private:
    std::deque<std::shared_ptr<gameplay::Drawable>> m_drawables{};
    std::deque<std::shared_ptr<gameplay::gl::Texture>> m_spriteTextures{};
    engine::Lighting m_lighting;

    void initDrawables(const level::Level& level);

protected:
    void nextFrame()
    {
        --negSpriteFrameId;

        if( m_drawables.empty() )
            return;

        m_drawables.emplace_back( m_drawables.front() );
        m_drawables.pop_front();
        setDrawable( m_drawables.front() );

        if( m_spriteTextures.empty() )
            return;

        m_spriteTextures.emplace_back( m_spriteTextures.front() );
        m_spriteTextures.pop_front();
    }

    void applyTransform()
    {
        glm::vec3 tr = pos.position.toRenderSystem() - pos.room->position.toRenderSystem();
        setLocalMatrix( translate( glm::mat4{1.0f}, tr ) * angle.toMatrix() );
    }

    size_t getLength() const
    {
        return m_drawables.size();
    }

public:
    explicit Particle(const std::string& id,
                      engine::TR1ItemId objectNumber,
                      const gsl::not_null<const loader::Room*>& room,
                      const level::Level& level)
            : Node{id}, pos{room}, object_number{objectNumber}
    {
        initDrawables( level );
    }

    explicit Particle(const std::string& id,
                      engine::TR1ItemId objectNumber,
                      const core::RoomBoundPosition& pos,
                      const level::Level& level)
            : Node{id}, pos{pos}, object_number{objectNumber}
    {
        initDrawables( level );
    }

    void updateLight()
    {
        m_lighting.updateStatic( shade );
    }

    virtual bool update(const level::Level& level) = 0;
};


class BloodSplatterParticle : public Particle
{
public:
    explicit BloodSplatterParticle(const core::RoomBoundPosition& pos,
                                   int16_t speed_,
                                   core::Angle angle_,
                                   const level::Level& level)
            : Particle{"bloodsplat", engine::TR1ItemId::Blood, pos, level}
    {
        speed = speed_;
        angle.Y = angle_;
    }

    bool update(const level::Level& level) override;
};


class SplashParticle : public Particle
{
public:
    explicit SplashParticle(const core::RoomBoundPosition& pos,
                            const level::Level& level,
                            bool waterfall)
            : Particle{"splash", engine::TR1ItemId::Splash, pos, level}
    {
        if( !waterfall )
        {
            speed = util::rand15( 128 );
            angle.Y = core::Angle( 2 * util::rand15s() );
        }
        else
        {
            this->pos.position.X += util::rand15s( loader::SectorSize );
            this->pos.position.Z += util::rand15s( loader::SectorSize );
        }
    }

    bool update(const level::Level& level) override;
};


class RicochetParticle : public Particle
{
public:
    explicit RicochetParticle(const core::RoomBoundPosition& pos,
                              const level::Level& level)
            : Particle{"ricochet", engine::TR1ItemId::Ricochet, pos, level}
    {
        timePerSpriteFrame = 4;

        int n = util::rand15( 3 );
        for( int i = 0; i < n; ++i )
            nextFrame();
    }

    bool update(const level::Level& /*level*/) override
    {
        --timePerSpriteFrame;
        if( timePerSpriteFrame == 0 )
        {
            return false;
        }

        applyTransform();
        return true;
    }
};


class BubbleParticle : public Particle
{
public:
    explicit BubbleParticle(const core::RoomBoundPosition& pos,
                            const level::Level& level)
            : Particle{"bubble", engine::TR1ItemId::Bubbles, pos, level}
    {
        speed = 10 + util::rand15( 6 );

        int n = util::rand15( 3 );
        for( int i = 0; i < n; ++i )
            nextFrame();
    }

    bool update(const level::Level& level) override;
};


class SparkleParticle : public Particle
{
public:
    explicit SparkleParticle(const core::RoomBoundPosition& pos,
                             const level::Level& level)
            : Particle{"sparkles", engine::TR1ItemId::Sparkles, pos, level}
    {
        timePerSpriteFrame = 0;
        negSpriteFrameId = 0;
    }

    bool update(const level::Level& /*level*/) override
    {
        ++timePerSpriteFrame;
        if( timePerSpriteFrame != 1 )
            return true;

        --negSpriteFrameId;
        timePerSpriteFrame = 0;
        return gsl::narrow<size_t>( -negSpriteFrameId ) < getLength();
    }
};


inline gsl::not_null<std::shared_ptr<Particle>> createBloodSplat(const level::Level& level,
                                                                 const core::RoomBoundPosition& pos,
                                                                 int16_t speed,
                                                                 core::Angle angle)
{
    auto particle = make_not_null_shared<BloodSplatterParticle>( pos, speed, angle, level );
    gameplay::setParent( particle, pos.room->node );
    return particle;
}
}