#include "cutsceneactors.h"

#include "level/level.h"

namespace engine
{
namespace items
{
CutsceneActor::CutsceneActor(const gsl::not_null<level::Level*>& level,
                             const std::string& name,
                             const gsl::not_null<const loader::Room*>& room,
                             const loader::Item& item,
                             const loader::SkeletalModelType& animatedModel)
        : ModelItemNode( level, name, room, item, true, animatedModel )
{
    activate();
    m_state.rotation.Y = 0_deg;
}

void CutsceneActor::update()
{
    m_state.rotation.Y = getLevel().m_cameraController->getEyeRotation().Y;
    m_state.position.position = getLevel().m_cameraController->getTRPosition().position;
    ModelItemNode::update();
}

void CutsceneActor4::update()
{
    ModelItemNode::update(); // NOLINT(bugprone-parent-virtual-call)
}
}
}
