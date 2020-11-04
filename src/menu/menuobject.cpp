#include "menuobject.h"

#include "engine/engine.h"
#include "engine/objects/laraobject.h"
#include "menuringtransform.h"

namespace menu
{
bool MenuObject::animate()
{
  if(meshAnimFrame == goalFrame)
  {
    updateMeshRenderMask();
    return false;
  }

  if(animStretchCounter != 0_frame)
  {
    animStretchCounter -= 1_frame;
  }
  else
  {
    animStretchCounter = animStretch;
    meshAnimFrame += animDirection;
    if(meshAnimFrame >= lastMeshAnimFrame)
    {
      meshAnimFrame = 0_frame;
    }
    else if(meshAnimFrame < 0_frame)
    {
      meshAnimFrame = lastMeshAnimFrame - 1_frame;
    }
  }
  updateMeshRenderMask();
  return true;
}

void MenuObject::updateMeshRenderMask()
{
  if(type == engine::TR1ItemId::PassportOpening)
  {
    if(meshAnimFrame <= 14_frame)
    {
      meshRenderMask = 0x57;
    }
    else if(meshAnimFrame <= 18_frame)
    {
      meshRenderMask = 0x5f;
    }
    else if(meshAnimFrame == 19_frame)
    {
      meshRenderMask = 0x5b;
    }
    else if(meshAnimFrame <= 23_frame)
    {
      meshRenderMask = 0x7b;
    }
    else if(meshAnimFrame <= 28_frame)
    {
      meshRenderMask = 0x3b;
    }
    else if(meshAnimFrame == 29_frame)
    {
      meshRenderMask = 0x13;
    }
  }
  else if(type == engine::TR1ItemId::Compass && (meshAnimFrame == 0_frame || meshAnimFrame >= 18_frame))
  {
    meshRenderMask = defaultMeshRenderMask;
  }
  else
  {
    meshRenderMask.set();
  }
}

void MenuObject::draw(const engine::Engine& engine,
                      const MenuRingTransform& ringTransform,
                      const core::Angle& ringItemAngle) const
{
  glm::mat4 nodeMatrix
    = ringTransform.getModelMatrix() * core::TRRotation{0_deg, ringItemAngle, 0_deg}.toMatrix()
      * glm::translate(glm::mat4{1.0f}, core::TRVec{ringTransform.radius, 0_len, 0_len}.toRenderSystem())
      * core::TRRotation{baseRotationX, 90_deg, 0_deg}.toMatrix()
      * glm::translate(glm::mat4{1.0f}, core::TRVec{0_len, 0_len, positionZ}.toRenderSystem())
      * core::TRRotation{rotationX, rotationY, 0_deg}.toMatrix();

  if(const auto& spriteSequence = engine.findSpriteSequenceForType(type))
  {
    BOOST_LOG_TRIVIAL(warning) << "Menu Sprite: " << toString(type);
    // TODO drawSprite
  }
  else if(const auto& obj = engine.findAnimatedModelForType(type))
  {
    // TODO avoid re-creating the model each time
    auto node = std::make_shared<engine::SkeletalModelNode>("menu-object", &engine, obj.get());
    node->setLocalMatrix(nodeMatrix);
    core::AnimStateId animState{0_as};
    engine::SkeletalModelNode::buildMesh(node, animState);
    node->setAnimation(animState, obj->animations, meshAnimFrame);
    for(size_t i = 0; i < static_cast<size_t>(obj->nMeshes); ++i)
    {
      node->setVisible(i, meshRenderMask.test(i));
    }
    node->rebuildMesh();

    if(type == engine::TR1ItemId::Compass)
    {
      const auto delta = (rotationY + engine.getObjectManager().getLara().m_state.rotation.Y + compassNeedleRotation
                          + util::rand15s(10_deg))
                         / 50;
      compassNeedleRotationMomentum = compassNeedleRotationMomentum * 19 / 20 - delta;
      compassNeedleRotation += compassNeedleRotationMomentum;
      node->patchBone(1, core::TRRotation{0_deg, compassNeedleRotation, 0_deg}.toMatrix());
    }

    node->updatePose();

    render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};
    context.setCurrentNode(node.get());
    node->getRenderable()->render(context);
  }
  else
  {
    BOOST_LOG_TRIVIAL(error) << "No sprite or model found for " << toString(type);
  }
}
} // namespace menu
