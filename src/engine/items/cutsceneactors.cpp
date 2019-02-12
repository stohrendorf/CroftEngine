#include "cutsceneactors.h"

#include "loader/file/level/level.h"

namespace engine
{
namespace items
{
CutsceneActor::CutsceneActor(const gsl::not_null<loader::file::level::Level*>& level,
                             const gsl::not_null<const loader::file::Room*>& room,
                             const loader::file::Item& item,
                             const loader::file::SkeletalModelType& animatedModel)
        : ModelItemNode{level, room, item, true, animatedModel}
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
