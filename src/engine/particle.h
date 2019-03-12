#pragma once

#include "core/angle.h"
#include "core/vec.h"
#include "items_tr1.h"
#include "Node.h"
#include "engine/lighting.h"

#include <memory>
#include <deque>

namespace engine
{
class Engine;


class Particle : public gameplay::Node, public audio::Emitter
{
public:
    core::RoomBoundPosition pos;
    core::TRRotation angle{0_deg, 0_deg, 0_deg};
    const core::TypeId object_number;
    core::Speed speed = 0_spd;
    core::Length fall_speed = 0_len;
    int16_t negSpriteFrameId = 0;
    int16_t timePerSpriteFrame = 0;
    int16_t shade = 4096;

private:
    std::deque<std::shared_ptr<gameplay::Drawable>> m_drawables{};
    std::deque<std::shared_ptr<gameplay::gl::Texture>> m_spriteTextures{};
    Lighting m_lighting;

    void initDrawables(const Engine& engine, float scale = 1);

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
                      const core::TypeId objectNumber,
                      const gsl::not_null<const loader::file::Room*>& room,
                      Engine& engine,
                      float scale = 1);

    explicit Particle(const std::string& id,
                      const TR1ItemId objectNumber,
                      const gsl::not_null<const loader::file::Room*>& room,
                      Engine& engine,
                      float scale = 1)
            : Particle( id, core::TypeId{static_cast<core::TypeId::type>(objectNumber)}, room, engine, scale )
    {}

    explicit Particle(const std::string& id,
                      const core::TypeId objectNumber,
                      const core::RoomBoundPosition& pos,
                      Engine& engine,
                      float scale = 1);

    explicit Particle(const std::string& id,
                      const TR1ItemId objectNumber,
                      const core::RoomBoundPosition& pos,
                      Engine& engine,
                      float scale = 1)
            : Particle( id, core::TypeId{static_cast<core::TypeId::type>(objectNumber)}, pos, engine, scale )
    {}

    void updateLight()
    {
        m_lighting.updateStatic( shade );
    }

    virtual bool update(Engine& engine) = 0;

    glm::vec3 getPosition() const final;
};


class BloodSplatterParticle : public Particle
{
public:
    explicit BloodSplatterParticle(const core::RoomBoundPosition& pos,
                                   const core::Speed speed_,
                                   const core::Angle angle_,
                                   Engine& engine)
            : Particle{"bloodsplat", TR1ItemId::Blood, pos, engine}
    {
        speed = speed_;
        angle.Y = angle_;
    }

    bool update(Engine& engine) override;
};


class SplashParticle : public Particle
{
public:
    explicit SplashParticle(const core::RoomBoundPosition& pos,
                            Engine& engine,
                            const bool waterfall)
            : Particle{"splash", TR1ItemId::Splash, pos, engine}
    {
        if( !waterfall )
        {
            speed = util::rand15( 128_spd, core::Speed::type() );
            angle.Y = core::Angle( 2 * util::rand15s() );
        }
        else
        {
            this->pos.position.X += util::rand15s( core::SectorSize, core::Length::type() );
            this->pos.position.Z += util::rand15s( core::SectorSize, core::Length::type() );
        }
    }

    bool update(Engine& engine) override;
};


class RicochetParticle : public Particle
{
public:
    explicit RicochetParticle(const core::RoomBoundPosition& pos,
                              Engine& engine)
            : Particle{"ricochet", TR1ItemId::Ricochet, pos, engine}
    {
        timePerSpriteFrame = 4;

        const int n = util::rand15( 3 );
        for( int i = 0; i < n; ++i )
            nextFrame();
    }

    bool update(Engine& /*engine*/) override
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
                            Engine& engine)
            : Particle{"bubble", TR1ItemId::Bubbles, pos, engine, 0.7f}
    {
        speed = 10_spd + util::rand15( 6_spd, core::Speed::type() );

        const int n = util::rand15( 3 );
        for( int i = 0; i < n; ++i )
            nextFrame();
    }

    bool update(Engine& engine) override;
};


class SparkleParticle : public Particle
{
public:
    explicit SparkleParticle(const core::RoomBoundPosition& pos,
                             Engine& engine)
            : Particle{"sparkles", TR1ItemId::Sparkles, pos, engine}
    {
    }

    bool update(Engine& /*engine*/) override
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
                              Engine& engine,
                              const core::Angle& yAngle)
            : Particle{"gunflare", TR1ItemId::Gunflare, pos, engine}
    {
        angle.Y = yAngle;
        timePerSpriteFrame = 3;
        shade = 4096;
    }

    bool update(Engine& /*engine*/) override
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
    explicit FlameParticle(const core::RoomBoundPosition& pos, Engine& engine)
            : Particle{"flame", TR1ItemId::Flame, pos, engine}
    {
        timePerSpriteFrame = 0;
        negSpriteFrameId = 0;
        shade = 4096;
    }

    bool update(Engine& engine) override;
};


inline gsl::not_null<std::shared_ptr<Particle>> createBloodSplat(Engine& engine,
                                                                 const core::RoomBoundPosition& pos,
                                                                 core::Speed speed,
                                                                 core::Angle angle)
{
    auto particle = std::make_shared<BloodSplatterParticle>( pos, speed, angle, engine );
    setParent( particle, pos.room->node );
    return particle;
}
}