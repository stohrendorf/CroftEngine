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
class FX : gameplay::Node
{
public:
    core::RoomBoundPosition pos;
    core::TRRotation angle;
    engine::TR1ItemId object_number;
    int16_t speed = 0;
    int16_t fall_speed = 0;
    int16_t negSpriteFrameId = 0;
    int16_t timePerSpriteFrame = 0;
    int16_t shade = 4096;

private:
    std::deque<std::shared_ptr<gameplay::Drawable>> m_drawables{};
    engine::Lighting m_lighting;

    void rotate()
    {
        BOOST_ASSERT( !m_drawables.empty() );
        m_drawables.push_back( m_drawables.front() );
        m_drawables.pop_front();
        setDrawable( m_drawables.front() );
    }

public:
    explicit FX(const std::string& id, const gsl::not_null<const loader::Room*>& room, const level::Level& level)
            : Node{id}, pos{room}
    {}

    explicit FX(const std::string& id, const core::RoomBoundPosition& pos, const level::Level& level)
            : Node{id}, pos{pos}
    {
    }

    void update()
    {
        m_lighting.bind( *this );
        m_lighting.updateStatic( shade );
    }

    void initDrawables(const level::Level& level, const gsl::not_null<std::shared_ptr<gameplay::Material>>& material);
};


inline std::shared_ptr<FX> createBloodSplat(const level::Level& level,
                                            const core::RoomBoundPosition& pos,
                                            int16_t speed,
                                            core::Angle angle)
{
    auto fx = std::make_shared<FX>( "bloodsplat", pos, level );
    fx->object_number = engine::TR1ItemId::Blood;
    fx->timePerSpriteFrame = 0;
    fx->speed = speed;
    fx->angle.Y = angle;

    return fx;
}

}
