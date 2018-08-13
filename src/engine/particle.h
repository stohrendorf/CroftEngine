#pragma once

#include "core/angle.h"
#include "core/coordinates.h"
#include "items_tr1.h"
#include "Node.h"
#include "level/level.h"
#include "engine/lighting.h"

#include <memory>
#include <deque>

namespace engine
{
class FX : public gameplay::Node
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

public:
    explicit FX(const std::string& id,
                engine::TR1ItemId objectNumber,
                const gsl::not_null<const loader::Room*>& room,
                const level::Level& level)
            : Node{id}, pos{room}, object_number{objectNumber}
    {
        initDrawables( level );
    }

    explicit FX(const std::string& id,
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

    virtual void update(const level::Level& level) = 0;
};


class BloodSplatterParticle : public FX
{
public:
    explicit BloodSplatterParticle(const core::RoomBoundPosition& pos,
                                   int16_t speed_,
                                   core::Angle angle_,
                                   const level::Level& level)
            : FX{"bloodsplat", engine::TR1ItemId::Blood, pos, level}
    {
        speed = speed_;
        angle.Y = angle_;
    }

    void update(const level::Level& level) override;
};


class SplashParticle : public FX
{
public:
    explicit SplashParticle(const core::RoomBoundPosition& pos,
                            const level::Level& level)
            : FX{"splash", engine::TR1ItemId::Splash, pos, level}
    {
        speed = util::rand15() * 7 / 8;
        angle.Y = core::Angle( 2 * (util::rand15() - 16384) );
    }

    void update(const level::Level& level) override;
};


class RicochetParticle : public FX
{
public:
    explicit RicochetParticle(const core::RoomBoundPosition& pos,
                              const level::Level& level)
            : FX{"splash", engine::TR1ItemId::Ricochet, pos, level}
    {
        timePerSpriteFrame = 4;

        int n = 3 * util::rand15() / 32768;
        for( int i = 0; i < n; ++i )
            nextFrame();
    }

    void update(const level::Level& level) override
    {
        --timePerSpriteFrame;
        if( timePerSpriteFrame == 0 )
        {
            // FIXME KillEffect(this);
        }

        applyTransform();
    }
};


inline gsl::not_null<std::shared_ptr<FX>> createBloodSplat(const level::Level& level,
                                                           const core::RoomBoundPosition& pos,
                                                           int16_t speed,
                                                           core::Angle angle)
{
    auto fx = make_not_null_shared<BloodSplatterParticle>( pos, speed, angle, level );
    gameplay::setParent( fx, pos.room->node );
    return fx;
}

}
