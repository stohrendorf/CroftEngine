#pragma once

#include "util/helpers.h"
#include "world/object.h"

#include <irrlicht.h>

namespace engine
{
class Engine;
}

namespace gui
{
class ItemNotifier
{
    TRACK_LIFETIME();

public:
    explicit ItemNotifier(engine::Engine* engine);

    void    start(world::ObjectId item, util::Duration time = util::Seconds(2));
    void    reset();
    void    animate();
    void    draw() const;

    void    setPos(irr::f32 X, irr::f32 Y);
    void    setRotation(irr::f32 X, irr::f32 Y);
    void    setSize(irr::f32 size);
    void    setRotateTime(util::Duration time);

private:
    engine::Engine* m_engine;

    bool    m_active = false;
    world::ObjectId m_item = 0;

    irr::core::vector2df m_absPos;

    irr::f32   m_posY;
    irr::f32   m_startPosX;
    irr::f32   m_endPosX;
    irr::f32   m_currPosX;

    irr::core::vector2df m_rotation{ 0,0 };
    irr::core::vector2df m_currentAngle{ 0,0 };
    irr::f32 m_radPerSecond = 0;

    irr::f32 m_size;

    util::Duration m_showTime;
    util::Duration m_currTime;
};
} // namespace gui
