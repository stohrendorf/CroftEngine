#include "puzzlehole.h"

#include "engine/laranode.h"
#include "engine/inputhandler.h"

namespace engine
{
namespace items
{
void PuzzleHole::collide(LaraNode& lara, CollisionInfo& /*collisionInfo*/)
{
    switch( m_state.type.get_as<TR1ItemId>() )
    {
        case TR1ItemId::PuzzleDone1:
        case TR1ItemId::PuzzleDone2:
        case TR1ItemId::PuzzleDone3:
        case TR1ItemId::PuzzleDone4:
            return;
        default:
            break;
    }

    static const InteractionLimits limits{
            core::BoundingBox{{-200_len, 0_len, 312_len},
                              {200_len,  0_len, 512_len}},
            core::TRRotation{-10_deg, -30_deg, -10_deg},
            core::TRRotation{10_deg, 30_deg, 10_deg}
    };

    if( lara.getCurrentAnimState() == LaraStateId::Stop )
    {
        if( !getEngine().getInputHandler().getInputState().action
            || lara.getHandStatus() != HandStatus::None
            || lara.m_state.falling
            || !limits.canInteract( m_state, lara.m_state ) )
            return;

        if( m_state.triggerState == TriggerState::Invisible )
        {
            lara.playSoundEffect( TR1SoundId::LaraNo );
            return;
        }

        bool hasPuzzlePiece = false;
        switch( m_state.type.get_as<TR1ItemId>() )
        {
            case TR1ItemId::PuzzleHole1:
                hasPuzzlePiece = getEngine().getInventory().tryTake( TR1ItemId::Puzzle1 );
                break;
            case TR1ItemId::PuzzleHole2:
                hasPuzzlePiece = getEngine().getInventory().tryTake( TR1ItemId::Puzzle2 );
                break;
            case TR1ItemId::PuzzleHole3:
                hasPuzzlePiece = getEngine().getInventory().tryTake( TR1ItemId::Puzzle3 );
                break;
            case TR1ItemId::PuzzleHole4:
                hasPuzzlePiece = getEngine().getInventory().tryTake( TR1ItemId::Puzzle4 );
                break;
            default:
                break;
        }
        if( !hasPuzzlePiece )
        {
            lara.playSoundEffect( TR1SoundId::LaraNo );
            return;
        }

        lara.alignForInteraction( core::TRVec{0_len, 0_len, 327_len}, m_state );

        do
        {
            lara.setGoalAnimState( LaraStateId::InsertPuzzle );
            lara.updateImpl();
        } while( lara.getCurrentAnimState() != LaraStateId::InsertPuzzle );

        lara.setGoalAnimState( LaraStateId::Stop );
        lara.setHandStatus( HandStatus::Grabbing );
        m_state.triggerState = TriggerState::Active;
    }
    else if( lara.getCurrentAnimState() == LaraStateId::InsertPuzzle && lara.m_state.frame_number == 3372_frame
             && limits.canInteract( m_state, lara.m_state ) )
    {
        TR1ItemId completeId;

        switch( m_state.type.get_as<TR1ItemId>() )
        {
            case TR1ItemId::PuzzleHole1:
                completeId = TR1ItemId::PuzzleDone1;
                break;
            case TR1ItemId::PuzzleHole2:
                completeId = TR1ItemId::PuzzleDone2;
                break;
            case TR1ItemId::PuzzleHole3:
                completeId = TR1ItemId::PuzzleDone3;
                break;
            case TR1ItemId::PuzzleHole4:
                completeId = TR1ItemId::PuzzleDone4;
                break;
            default:
                BOOST_THROW_EXCEPTION( std::runtime_error( "Invalid puzzle ID" ) );
        }

        const auto& model = getEngine().findAnimatedModelForType( completeId );
        Expects( model != nullptr );

        const auto parent = m_skeleton->getParent().lock();
        setParent( m_skeleton, nullptr );

        m_skeleton = std::make_shared<SkeletalModelNode>( toString( completeId ), &getEngine(), *model );
        m_skeleton->setAnimation( m_state, model->animations, model->animations->firstFrame );
        for( gsl::index boneIndex = 0; boneIndex < model->models.size(); ++boneIndex )
        {
            auto node = std::make_shared<render::scene::Node>( "bone:" + std::to_string( boneIndex ) );
            node->setDrawable( model->models[boneIndex].get() );
            addChild( getNode(), node );
        }

        setParent( m_skeleton, parent );

        m_state.type = completeId;
        ModelItemNode::update();
    }
}
}
}
