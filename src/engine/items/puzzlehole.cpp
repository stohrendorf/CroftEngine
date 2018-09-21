#include "puzzlehole.h"

#include "engine/laranode.h"
#include "stubitem.h"

namespace engine
{
namespace items
{
void PuzzleHole::collide(engine::LaraNode& lara, engine::CollisionInfo& /*collisionInfo*/)
{
    switch( m_state.object_number )
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
            core::BoundingBox{{-200, 0, 312},
                              {200,  0, 512}},
            core::TRRotation{-10_deg, -30_deg, -10_deg},
            core::TRRotation{10_deg, 30_deg, 10_deg}
    };

    if( lara.getCurrentAnimState() == loader::LaraStateId::Stop )
    {
        if( !getLevel().m_inputHandler->getInputState().action
            || lara.getHandStatus() != engine::HandStatus::None
            || lara.m_state.falling
            || !limits.canInteract( m_state, lara.m_state ) )
            return;

        if( m_state.triggerState == engine::items::TriggerState::Invisible )
        {
            lara.playSoundEffect( 2 );
            return;
        }

        bool hasPuzzlePiece = false;
        switch( m_state.object_number )
        {
            case engine::TR1ItemId::PuzzleHole1:
                hasPuzzlePiece = getLevel().takeInventoryItem( engine::TR1ItemId::Puzzle1 );
                break;
            case engine::TR1ItemId::PuzzleHole2:
                hasPuzzlePiece = getLevel().takeInventoryItem( engine::TR1ItemId::Puzzle2 );
                break;
            case engine::TR1ItemId::PuzzleHole3:
                hasPuzzlePiece = getLevel().takeInventoryItem( engine::TR1ItemId::Puzzle3 );
                break;
            case engine::TR1ItemId::PuzzleHole4:
                hasPuzzlePiece = getLevel().takeInventoryItem( engine::TR1ItemId::Puzzle4 );
                break;
            default:
                break;
        }
        if( !hasPuzzlePiece )
        {
            lara.playSoundEffect( 2 );
            return;
        }

        lara.alignForInteraction( core::TRVec{0, 0, 327}, m_state );

        do
        {
            lara.setGoalAnimState( LaraStateId::InsertPuzzle );
            lara.updateImpl();
        } while( lara.getCurrentAnimState() != LaraStateId::InsertPuzzle );

        lara.setGoalAnimState( loader::LaraStateId::Stop );
        lara.setHandStatus( engine::HandStatus::Grabbing );
        m_state.triggerState = engine::items::TriggerState::Active;
    }
    else if( lara.getCurrentAnimState() == loader::LaraStateId::InsertPuzzle && lara.m_state.frame_number == 3372
             && limits.canInteract( m_state, lara.m_state ) )
    {
        TR1ItemId completeId;

        switch( m_state.object_number )
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

        const auto& model = getLevel().findAnimatedModelForType( completeId );
        Expects( model != nullptr );

        const auto parent = gsl::make_not_null( m_skeleton->getParent().lock() );
        setParent( gsl::make_not_null( m_skeleton ), nullptr );

        m_skeleton = std::make_shared<SkeletalModelNode>( toString( completeId ), gsl::make_not_null( &getLevel() ),
                                                          *model );
        m_skeleton->setAnimation( m_state,
                                  gsl::make_not_null( model->animation ),
                                  model->animation->firstFrame );
        for( gsl::index boneIndex = 0; boneIndex < model->models.size(); ++boneIndex )
        {
            auto node = make_not_null_shared<gameplay::Node>( "bone:" + std::to_string( boneIndex ) );
            node->setDrawable( model->models[boneIndex].get() );
            addChild( gsl::make_not_null( getNode() ), node );
        }

        setParent( gsl::make_not_null( m_skeleton ), parent );

        m_state.object_number = completeId;
        ModelItemNode::update();
    }
}
}
}
