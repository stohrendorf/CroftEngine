#include "itemnotifier.h"

#include "engine/engine.h"
#include "engine/system.h"
#include "gui.h"
#include "world/entity.h"

namespace gui
{
ItemNotifier::ItemNotifier(engine::Engine* engine)
    : m_engine(engine)
{
    setPos(850, 850);
    // SetRot(0, 0);
    setRotation(util::Rad180, util::Rad180 + util::Rad90);
    // SetSize(1.0);
    setSize(128.0);
    // SetRotateTime(1000.0);
    setRotateTime(util::Seconds(2.5));
}

void ItemNotifier::start(world::ObjectId item, util::Duration time)
{
    reset();

    m_item = item;
    m_showTime = time;
    m_active = true;
}

void ItemNotifier::animate()
{
    if(!m_active)
    {
        return;
    }

    if(!util::fuzzyZero(m_radPerSecond))
    {
        m_currentAngle.X = std::fmod(m_currentAngle.X + m_engine->getFrameTimeSecs() * m_radPerSecond, util::Rad360);
    }

    if(util::fuzzyZero(m_currTime.count()))
    {
        irr::f32 step = (m_currPosX - m_endPosX) * m_engine->getFrameTimeSecs() * 4.0f;
        step = std::max(0.5f, step);

        m_currPosX = std::min(m_currPosX - step, m_endPosX);

        if(util::fuzzyEqual(m_currPosX, m_endPosX))
            m_currTime += m_engine->getFrameTime();
    }
    else if(m_currTime < m_showTime)
    {
        m_currTime += m_engine->getFrameTime();
    }
    else
    {
        irr::f32 step = (m_currPosX - m_endPosX) * m_engine->getFrameTimeSecs() * 4;
        step = std::max(0.5f, step);

        m_currPosX = std::min(m_currPosX + step, m_startPosX);

        if(m_currPosX == m_startPosX)
            reset();
    }
}

void ItemNotifier::reset()
{
    m_active = false;
    m_currTime = util::Duration(0);
    m_currentAngle = irr::core::vector2df{ 0,0 };

    m_endPosX = static_cast<irr::f32>(m_engine->m_screenInfo.w) / ScreenMeteringResolution * m_absPos.X;
    m_posY = static_cast<float>(m_engine->m_screenInfo.h) / ScreenMeteringResolution * m_absPos.Y;
    m_currPosX = m_engine->m_screenInfo.w + static_cast<float>(m_engine->m_screenInfo.w) / NotifierOffscreenDivider * m_size;
    m_startPosX = m_currPosX;    // Equalize current and start positions.
}

void ItemNotifier::draw() const
{
    if(!m_active)
        return;

    auto item = m_engine->m_world.getBaseItemByID(m_item);
    if(!item)
        return;

    const world::animation::AnimationId anim = item->getSkeleton().getCurrentAnimationId();
    const auto frame = item->getSkeleton().getCurrentFrame();

    item->getSkeleton().setCurrentAnimationId(0);
    item->getSkeleton().setCurrentFrame(0);

    item->getSkeleton().itemFrame(util::Duration(0));
    irr::core::matrix4 matrix;
    matrix.setTranslation({ m_currPosX, m_posY, -2048.0 });
    matrix *= irr::core::matrix4().setRotationAxisRadians(m_currentAngle.X + m_rotation.X, { 0,1,0 });
    matrix *= irr::core::matrix4().setRotationAxisRadians(m_currentAngle.Y + m_rotation.Y, { 1,0,0 });
    render::renderItem(item->getSkeleton(), m_size, matrix, m_engine->m_gui.m_guiProjectionMatrix);

    item->getSkeleton().setCurrentAnimationId(anim);
    item->getSkeleton().setCurrentFrame(frame);
}

void ItemNotifier::setPos(irr::f32 X, irr::f32 Y)
{
    m_absPos = irr::core::vector2df{ X, 1000.0f - Y };
}

void ItemNotifier::setRotation(irr::f32 X, irr::f32 Y)
{
    m_rotation = irr::core::vector2df{ X, Y };
}

void ItemNotifier::setSize(irr::f32 size)
{
    m_size = size;
}

void ItemNotifier::setRotateTime(util::Duration time)
{
    m_radPerSecond = util::Seconds(1) / time * util::Rad360;
}
} // namespace gui
