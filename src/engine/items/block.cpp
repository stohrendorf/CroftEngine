#include "block.h"

#include "engine/laranode.h"
#include "level/level.h"
#include "core/boundingbox.h"

namespace engine
{
namespace items
{
void Block::collide(LaraNode& lara, CollisionInfo& /*collisionInfo*/)
{
    if( !getLevel().m_inputHandler->getInputState().action
        || m_state.triggerState == TriggerState::Active
        || lara.m_state.falling
        || lara.m_state.position.position.Y != m_state.position.position.Y )
    {
        return;
    }

    static const InteractionLimits limits{
            core::BoundingBox{{-300_len, 0_len, -692_len},
                              {200_len,  0_len, -512_len}},
            {-10_deg, -30_deg, -10_deg},
            {+10_deg, +30_deg, +10_deg}
    };

    auto axis = axisFromAngle( lara.m_state.rotation.Y, 45_deg );
    Expects( axis.is_initialized() );

    if( lara.getCurrentAnimState() == loader::LaraStateId::Stop )
    {
        if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Null
            || lara.getHandStatus() != HandStatus::None )
        {
            return;
        }

        const core::Angle y = alignRotation( *axis );
        m_state.rotation.Y = y;

        if( !limits.canInteract( m_state, lara.m_state ) )
        {
            return;
        }

        lara.m_state.rotation.Y = y;

        core::Length core::TRVec::*vp;
        core::Length d = 0_len;
        switch( *axis )
        {
            case core::Axis::PosZ:
                d = core::SectorSize - core::DefaultCollisionRadius;
                vp = &core::TRVec::Z;
                break;
            case core::Axis::PosX:
                d = core::SectorSize - core::DefaultCollisionRadius;
                vp = &core::TRVec::X;
                break;
            case core::Axis::NegZ:
                d = core::DefaultCollisionRadius;
                vp = &core::TRVec::Z;
                break;
            case core::Axis::NegX:
                d = core::DefaultCollisionRadius;
                vp = &core::TRVec::X;
                break;
            default:
                BOOST_THROW_EXCEPTION( std::domain_error( "Invalid axis" ) );
        }

        lara.m_state.position.position.*vp =
                (lara.m_state.position.position.*vp / core::SectorSize) * core::SectorSize + d;

        lara.setGoalAnimState( loader::LaraStateId::PushableGrab );
        lara.updateImpl();
        if( lara.getCurrentAnimState() == loader::LaraStateId::PushableGrab )
        {
            lara.setHandStatus( HandStatus::Grabbing );
        }
        return;
    }

    if( lara.getCurrentAnimState() != loader::LaraStateId::PushableGrab
        || lara.m_state.frame_number != 2091_frame
        || !limits.canInteract( m_state, lara.m_state ) )
    {
        return;
    }

    if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
    {
        if( !canPushBlock( core::SectorSize, *axis ) )
        {
            return;
        }

        m_state.goal_anim_state = 2_as;
        lara.setGoalAnimState( loader::LaraStateId::PushablePush );
    }
    else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward )
    {
        if( !canPullBlock( core::SectorSize, *axis ) )
        {
            return;
        }

        m_state.goal_anim_state = 3_as;
        lara.setGoalAnimState( loader::LaraStateId::PushablePull );
    }
    else
    {
        return;
    }

    activate();
    loader::Room::patchHeightsForBlock( *this, core::SectorSize );
    m_state.triggerState = TriggerState::Active;

    ModelItemNode::update();
    getLevel().m_lara->updateImpl();
}

void Block::update()
{
    if( m_state.activationState.isOneshot() )
    {
        loader::Room::patchHeightsForBlock( *this, core::SectorSize );
        kill();
        return;
    }

    ModelItemNode::update();

    auto pos = m_state.position;
    auto sector = level::Level::findRealFloorSector( pos );
    const auto height = HeightInfo::fromFloor( sector, pos.position, getLevel().m_itemNodes ).y;
    if( height > pos.position.Y )
    {
        m_state.falling = true;
    }
    else if( m_state.falling )
    {
        pos.position.Y = height;
        m_state.position.position = pos.position;
        m_state.falling = false;
        m_state.triggerState = TriggerState::Deactivated;
        getLevel().dinoStompEffect( *this );
        playSoundEffect( TR1SoundId::TRexFootstep );
        applyTransform(); // needed for properly placing geometry on floor
    }

    setCurrentRoom( pos.room );

    if( m_state.triggerState != TriggerState::Deactivated )
    {
        return;
    }

    m_state.triggerState = TriggerState::Inactive;
    deactivate();
    loader::Room::patchHeightsForBlock( *this, -core::SectorSize );
    pos = m_state.position;
    sector = level::Level::findRealFloorSector( pos );
    getLevel().m_lara->handleCommandSequence(
            HeightInfo::fromFloor( sector, pos.position, getLevel().m_itemNodes ).lastCommandSequenceOrDeath, true );
}

bool Block::isOnFloor(const core::Length height) const
{
    const auto sector = level::Level::findRealFloorSector( m_state.position.position, m_state.position.room );
    return sector->floorHeight == -core::HeightLimit || sector->floorHeight == m_state.position.position.Y - height;
}

bool Block::canPushBlock(const core::Length height, const core::Axis axis) const
{
    if( !isOnFloor( height ) )
    {
        return false;
    }

    auto pos = m_state.position.position;
    switch( axis )
    {
        case core::Axis::PosZ:
            pos.Z += core::SectorSize;
            break;
        case core::Axis::PosX:
            pos.X += core::SectorSize;
            break;
        case core::Axis::NegZ:
            pos.Z -= core::SectorSize;
            break;
        case core::Axis::NegX:
            pos.X -= core::SectorSize;
            break;
        default:
            break;
    }

    CollisionInfo tmp;
    tmp.facingAxis = axis;
    tmp.collisionRadius = 500_len;
    if( tmp.checkStaticMeshCollisions( pos, 2 * tmp.collisionRadius, getLevel() ) )
    {
        return false;
    }

    const auto targetSector = level::Level::findRealFloorSector( pos, m_state.position.room );
    if( targetSector->floorHeight != pos.Y )
    {
        return false;
    }

    pos.Y -= height;
    return pos.Y >= level::Level::findRealFloorSector(
            pos, m_state.position.room )->ceilingHeight;
}

bool Block::canPullBlock(const core::Length height, const core::Axis axis) const
{
    if( !isOnFloor( height ) )
    {
        return false;
    }

    auto pos = m_state.position.position;
    switch( axis )
    {
        case core::Axis::PosZ:
            pos.Z -= core::SectorSize;
            break;
        case core::Axis::PosX:
            pos.X -= core::SectorSize;
            break;
        case core::Axis::NegZ:
            pos.Z += core::SectorSize;
            break;
        case core::Axis::NegX:
            pos.X += core::SectorSize;
            break;
        default:
            break;
    }

    auto room = m_state.position.room;
    auto sector = level::Level::findRealFloorSector( pos, &room );

    CollisionInfo tmp;
    tmp.facingAxis = axis;
    tmp.collisionRadius = 500_len;
    if( tmp.checkStaticMeshCollisions( pos, 2 * tmp.collisionRadius, getLevel() ) )
    {
        return false;
    }

    if( sector->floorHeight != pos.Y )
    {
        return false;
    }

    auto topPos = pos;
    topPos.Y -= height;
    const auto topSector = level::Level::findRealFloorSector( topPos, m_state.position.room );
    if( topPos.Y < topSector->ceilingHeight )
    {
        return false;
    }

    auto laraPos = pos;
    switch( axis )
    {
        case core::Axis::PosZ:
            laraPos.Z -= core::SectorSize;
            break;
        case core::Axis::PosX:
            laraPos.X -= core::SectorSize;
            break;
        case core::Axis::NegZ:
            laraPos.Z += core::SectorSize;
            break;
        case core::Axis::NegX:
            laraPos.X += core::SectorSize;
            break;
        default:
            break;
    }

    sector = level::Level::findRealFloorSector( laraPos, &room );
    if( sector->floorHeight != pos.Y )
    {
        return false;
    }

    laraPos.Y -= core::LaraWalkHeight;
    sector = level::Level::findRealFloorSector( laraPos, &room );
    if( laraPos.Y < sector->ceilingHeight )
    {
        return false;
    }

    laraPos = getLevel().m_lara->m_state.position.position;
    switch( axis )
    {
        case core::Axis::PosZ:
            laraPos.Z -= core::SectorSize;
            tmp.facingAxis = core::Axis::NegZ;
            break;
        case core::Axis::PosX:
            laraPos.X -= core::SectorSize;
            tmp.facingAxis = core::Axis::NegX;
            break;
        case core::Axis::NegZ:
            laraPos.Z += core::SectorSize;
            tmp.facingAxis = core::Axis::PosZ;
            break;
        case core::Axis::NegX:
            laraPos.X += core::SectorSize;
            tmp.facingAxis = core::Axis::PosX;
            break;
        default:
            break;
    }
    tmp.collisionRadius = core::DefaultCollisionRadius;

    return !tmp.checkStaticMeshCollisions( laraPos, core::LaraWalkHeight, getLevel() );
}

void Block::load(const YAML::Node& n)
{
    if( m_state.triggerState != TriggerState::Invisible )
        loader::Room::patchHeightsForBlock( *this, core::SectorSize );

    ModelItemNode::load( n );

    if( m_state.triggerState != TriggerState::Invisible )
        loader::Room::patchHeightsForBlock( *this, -core::SectorSize );
}
}
}
