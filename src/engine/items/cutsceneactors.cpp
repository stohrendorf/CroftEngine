#include "cutsceneactors.h"

#include "engine/engine.h"

namespace engine
{
namespace items
{
CutsceneActor::CutsceneActor(const gsl::not_null<Engine*>& engine,
                             const gsl::not_null<const loader::file::Room*>& room,
                             const loader::file::Item& item,
                             const loader::file::SkeletalModelType& animatedModel)
    : ModelItemNode{ engine, room, item, true, animatedModel }
{
    activate();
    m_state.rotation.Y = 0_deg;
}

void CutsceneActor::update()
{
    m_state.rotation.Y = getEngine().getCameraController().getEyeRotation().Y;
    m_state.position.position = getEngine().getCameraController().getTRPosition().position;
    ModelItemNode::update();
}

void CutsceneActor4::update()
{
    ModelItemNode::update(); // NOLINT(bugprone-parent-virtual-call)
}
}
}
