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
class Particle : public gameplay::Node, public audio::Emitter
{
public:
    core::RoomBoundPosition pos;
    core::TRRotation angle{0_deg, 0_deg, 0_deg};
    const TR1ItemId object_number;
    core::Length speed = 0_len;
    core::Length fall_speed = 0_len;
    int16_t negSpriteFrameId = 0;
    int16_t timePerSpriteFrame = 0;
    int16_t shade = 4096;

private:
    std::deque<std::shared_ptr<gameplay::Drawable>> m_drawables{};
    std::deque<std::shared_ptr<gameplay::gl::Texture>> m_spriteTextures{};
    Lighting m_lighting;

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
        const glm::vec3 tr = pos.position.toRenderSystem() - pos.room->position.toRenderSystem();
        setLocalMatrix( translate( glm::mat4{1.0f}, tr ) * angle.toMatrix() );
    }

    size_t getLength() const
    {
        return m_drawables.size();
    }

public:
    explicit Particle(const std::string& id,
                      const TR1ItemId objectNumber,
                      const gsl::not_null<const loader::Room*>& room,
                      level::Level& level);

    explicit Particle(const std::string& id,
                      const TR1ItemId objectNumber,
                      const core::RoomBoundPosition& pos,
                      level::Level& level);

    void updateLight()
    {
        m_lighting.updateStatic( shade );
    }

    virtual bool update(level::Level& level) = 0;

    glm::vec3 getPosition() const final;
};


class BloodSplatterParticle : public Particle
{
public:
    explicit BloodSplatterParticle(const core::RoomBoundPosition& pos,
                                   const core::Length speed_,
                                   const core::Angle angle_,
                                   level::Level& level)
            : Particle{"bloodsplat", TR1ItemId::Blood, pos, level}
    {
        speed = speed_;
        angle.Y = angle_;
    }

    bool update(level::Level& level) override;
};


class SplashParticle : public Particle
{
public:
    explicit SplashParticle(const core::RoomBoundPosition& pos,
                            level::Level& level,
                            const bool waterfall)
            : Particle{"splash", TR1ItemId::Splash, pos, level}
    {
        if( !waterfall )
        {
            speed = util::rand15( 128_len );
            angle.Y = core::Angle( 2 * util::rand15s() );
        }
        else
        {
            this->pos.position.X += util::rand15s( core::SectorSize );
            this->pos.position.Z += util::rand15s( core::SectorSize );
        }
    }

    bool update(level::Level& level) override;
};


class RicochetParticle : public Particle
{
public:
    explicit RicochetParticle(const core::RoomBoundPosition& pos,
                              level::Level& level)
            : Particle{"ricochet", TR1ItemId::Ricochet, pos, level}
    {
        timePerSpriteFrame = 4;

        const int n = util::rand15( 3 );
        for( int i = 0; i < n; ++i )
            nextFrame();
    }

    bool update(level::Level& /*level*/) override
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
                            level::Level& level)
            : Particle{"bubble", TR1ItemId::Bubbles, pos, level}
    {
        speed = 10_len + util::rand15( 6_len );

        const int n = util::rand15( 3 );
        for( int i = 0; i < n; ++i )
            nextFrame();
    }

    bool update(level::Level& level) override;
};


class SparkleParticle : public Particle
{
public:
    explicit SparkleParticle(const core::RoomBoundPosition& pos,
                             level::Level& level)
            : Particle{"sparkles", TR1ItemId::Sparkles, pos, level}
    {
    }

    bool update(level::Level& /*level*/) override
    {
        ++timePerSpriteFrame;
        if( timePerSpriteFrame != 1 )
            return true;

        --negSpriteFrameId;
        timePerSpriteFrame = 0;
        return gsl::narrow<size_t>( -negSpriteFrameId ) < getLength();
    }
};


class GunflareParticle : public Particle
{
public:
    explicit GunflareParticle(const core::RoomBoundPosition& pos,
                              level::Level& level,
                              const core::Angle& yAngle)
            : Particle{"gunflare", TR1ItemId::Gunflare, pos, level}
    {
        angle.Y = yAngle;
        timePerSpriteFrame = 3;
        shade = 4096;
    }

    bool update(level::Level& /*level*/) override
    {
        --timePerSpriteFrame;
        if( timePerSpriteFrame == 0 )
            return false;

        angle.Z = util::rand15s( +180_deg );
        return true;
    }
};


class FlameParticle : public Particle
{
public:
    explicit FlameParticle(const core::RoomBoundPosition& pos,
                           level::Level& level)
            : Particle{"flame", TR1ItemId::Flame, pos, level}
    {
        timePerSpriteFrame = 0;
        negSpriteFrameId = 0;
        shade = 4096;
    }

    bool update(level::Level& level) override;
};


inline gsl::not_null<std::shared_ptr<Particle>> createBloodSplat(level::Level& level,
                                                                 const core::RoomBoundPosition& pos,
                                                                 core::Length speed,
                                                                 core::Angle angle)
{
    auto particle = std::make_shared<BloodSplatterParticle>( pos, speed, angle, level );
    setParent( particle, pos.room->node );
    return particle;
}
}