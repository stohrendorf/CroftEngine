#include "itemcontroller.h"

#include "level.h"
#include "animationcontroller.h"

uint16_t ItemController::getCurrentAnimState() const
{
    return m_dispatcher->getCurrentAnimState();
}

bool ItemController::handleTRTransitions()
{
    return m_dispatcher->handleTRTransitions();
}

void ItemController::applyRotation()
{
    //! @todo This is horribly inefficient code, but it properly converts ZXY angles to XYZ angles.
    irr::core::quaternion q;
    q.makeIdentity();
    q *= irr::core::quaternion().fromAngleAxis(getRotation().Y.toRad(), { 0,1,0 });
    q *= irr::core::quaternion().fromAngleAxis(getRotation().X.toRad(), { -1,0,0 });
    q *= irr::core::quaternion().fromAngleAxis(getRotation().Z.toRad(), { 0,0,-1 });

    irr::core::vector3df euler;
    q.toEuler(euler);
    m_sceneNode->setRotation(euler * 180 / irr::core::PI);
}

void ItemController::setTargetState(uint16_t st)
{
    m_dispatcher->setTargetState(st);
}

uint16_t ItemController::getTargetState() const
{
    return m_dispatcher->getTargetState();
}

void ItemController::playAnimation(uint16_t anim, const boost::optional<irr::u32>& firstFrame)
{
    m_dispatcher->playLocalAnimation(anim, firstFrame);
}

irr::u32 ItemController::getCurrentFrame() const
{
    return m_dispatcher->getCurrentFrame();
}

irr::u32 ItemController::getAnimEndFrame() const
{
    return m_dispatcher->getAnimEndFrame();
}

irr::core::aabbox3di ItemController::getBoundingBox() const
{
    return m_dispatcher->getBoundingBox();
}

void ItemController::setCurrentRoom(const loader::Room* newRoom)
{
    if(newRoom == m_currentRoom)
        return;

    BOOST_LOG_TRIVIAL(debug) << "Room switch to " << newRoom->node->getName();
    if(newRoom == nullptr)
    {
        BOOST_LOG_TRIVIAL(fatal) << "No room to switch to. Matching rooms by position:";
        for(size_t i = 0; i < m_level->m_rooms.size(); ++i)
        {
            const loader::Room& room = m_level->m_rooms[i];
            if(room.node->getTransformedBoundingBox().isPointInside(m_sceneNode->getAbsolutePosition()))
            {
                BOOST_LOG_TRIVIAL(fatal) << "  - " << i;
            }
        }
        return;
    }

    m_currentRoom = newRoom;
    for(irr::u32 i = 0; i < m_sceneNode->getMaterialCount(); ++i)
    {
        irr::video::SMaterial& material = m_sceneNode->getMaterial(i);
        const auto col = m_currentRoom->lightColor.toSColor(m_currentRoom->darkness / 8191.0f / 4);
        material.EmissiveColor = col;
        material.AmbientColor = col;
    }
}

uint16_t ItemController::getCurrentAnimationId() const
{
    return m_dispatcher->getCurrentAnimationId();
}

float ItemController::calculateAnimFloorSpeed() const
{
    return m_dispatcher->calculateFloorSpeed();
}

int ItemController::getAnimAccelleration() const
{
    return m_dispatcher->getAccelleration();
}
