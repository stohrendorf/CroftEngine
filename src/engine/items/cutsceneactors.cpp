#include "cutsceneactors.h"

#include "level/level.h"

namespace engine
{
namespace items
{
CutsceneActor1::CutsceneActor1(const gsl::not_null<level::Level*>& level, const std::string& name,
                               const gsl::not_null<const loader::Room*>& room, const loader::Item& item,
                               const loader::SkeletalModelType& animatedModel)
        : ModelItemNode( level, name, room, item, true, animatedModel )
{
    activate();
    m_state.rotation.Y = 0_deg;
    if( false ) // FIXME cinematic_level == CUTSCENE2 || cinematic_level == CUTSCENE4 )
    {
        const auto& laraPistol = level->findAnimatedModelForType( engine::TR1ItemId::LaraPistolsAnim );
        getNode()->getChild( 1 )->setDrawable( laraPistol->models[1].get() );
        getNode()->getChild( 4 )->setDrawable( laraPistol->models[4].get() );
    }
}

CutsceneActor2::CutsceneActor2(const gsl::not_null<level::Level*>& level, const std::string& name,
                               const gsl::not_null<const loader::Room*>& room, const loader::Item& item,
                               const loader::SkeletalModelType& animatedModel)
        : ModelItemNode( level, name, room, item, true, animatedModel )
{
    activate();
    m_state.rotation.Y = 0_deg;
}

CutsceneActor3::CutsceneActor3(const gsl::not_null<level::Level*>& level, const std::string& name,
                               const gsl::not_null<const loader::Room*>& room, const loader::Item& item,
                               const loader::SkeletalModelType& animatedModel)
        : ModelItemNode( level, name, room, item, true, animatedModel )
{
    activate();
    m_state.rotation.Y = 0_deg;
}

CutsceneActor4::CutsceneActor4(const gsl::not_null<level::Level*>& level, const std::string& name,
                               const gsl::not_null<const loader::Room*>& room, const loader::Item& item,
                               const loader::SkeletalModelType& animatedModel)
        : ModelItemNode( level, name, room, item, true, animatedModel )
{
    activate();
    m_state.rotation.Y = 0_deg;
}

void CutsceneActor1::update()
{
    m_state.rotation.Y = getLevel().m_cameraController->getTargetRotation().Y;
    m_state.position.position = getLevel().m_cameraController->getTRPosition().position;
    ModelItemNode::update();
}

void CutsceneActor2::update()
{
    m_state.rotation.Y = getLevel().m_cameraController->getTargetRotation().Y;
    m_state.position.position = getLevel().m_cameraController->getTRPosition().position;
    ModelItemNode::update();
}

void CutsceneActor3::update()
{
    m_state.rotation.Y = getLevel().m_cameraController->getTargetRotation().Y;
    m_state.position.position = getLevel().m_cameraController->getTRPosition().position;
    ModelItemNode::update();
}

void CutsceneActor4::update()
{
    ModelItemNode::update();
}
}
}
