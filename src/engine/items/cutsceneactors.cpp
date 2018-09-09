#include "cutsceneactors.h"

#include "level/level.h"

namespace engine
{
namespace items
{
CutsceneActor::CutsceneActor(const gsl::not_null<level::Level*>& level, const std::string& name,
                             const gsl::not_null<const loader::Room*>& room, const loader::Item& item,
                             const loader::SkeletalModelType& animatedModel)
        : ModelItemNode( level, name, room, item, true, animatedModel )
{
    activate();
    m_state.rotation.Y = 0_deg;
}

void CutsceneActor::update()
{
    m_state.rotation.Y = getLevel().m_cameraController->getTargetRotation().Y;
    m_state.position.position = getLevel().m_cameraController->getTRPosition().position;
    ModelItemNode::update();
}

CutsceneActor1::CutsceneActor1(const gsl::not_null<level::Level*>& level, const std::string& name,
                               const gsl::not_null<const loader::Room*>& room, const loader::Item& item,
                               const loader::SkeletalModelType& animatedModel)
        : CutsceneActor( level, name, room, item, animatedModel )
{
    if( false ) // FIXME cinematic_level == CUTSCENE2 || cinematic_level == CUTSCENE4 )
    {
        const auto& laraPistol = level->findAnimatedModelForType( engine::TR1ItemId::LaraPistolsAnim );
        getNode()->getChild( 1 )->setDrawable( laraPistol->models[1].get() );
        getNode()->getChild( 4 )->setDrawable( laraPistol->models[4].get() );
    }
}

void CutsceneActor4::update()
{
    ModelItemNode::update(); // NOLINT(bugprone-parent-virtual-call)
}
}
}
