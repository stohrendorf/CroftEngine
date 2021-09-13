#include "shatter.h"

#include "engine/items_tr1.h"
#include "engine/particle.h"
#include "engine/presenter.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/rendermeshdata.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "modelobject.h"
#include "render/scene/mesh.h"

#include <boost/log/trivial.hpp>

namespace engine::objects
{
bool shatterModel(ModelObject& object, const std::bitset<32>& meshMask, const core::Length& damageRadius)
{
  object.getSkeleton()->updatePose();
  const bool isTorsoBoss = object.m_state.type == TR1ItemId::TorsoBoss;

  const auto modelSourceType
    = object.m_state.type == TR1ItemId::WalkingMutant1 || object.m_state.type == TR1ItemId::WalkingMutant2
        ? TR1ItemId::FlyingMutant
        : object.m_state.type;
  const auto& modelType = object.getWorld().findAnimatedModelForType(modelSourceType);
  Expects(modelType != nullptr);
  Expects(modelType->bones.size() == object.getSkeleton()->getBoneCount());
  BOOST_LOG_TRIVIAL(trace) << "Shatter model: " << modelType->bones.size() << " meshes";

  for(size_t i = 0; i < modelType->bones.size(); ++i)
  {
    if(!meshMask.test(i) || !object.getSkeleton()->isVisible(i))
    {
      BOOST_LOG_TRIVIAL(trace) << "Shatter model: mesh " << i << " skipped";
      continue;
    }

    object.getSkeleton()->setVisible(i, false);
    object.getSkeleton()->rebuildMesh();
    world::RenderMeshDataCompositor compositor;
    compositor.append(*modelType->bones[i].mesh);
    auto particle = std::make_shared<MeshShrapnelParticle>(
      Location{object.m_state.location.room, core::TRVec{object.getSkeleton()->getMeshPartTranslationWorld(i)}},
      object.getWorld(),
      compositor.toMesh(*object.getWorld().getPresenter().getMaterialManager(), false, {}),
      isTorsoBoss,
      damageRadius);
    particle->negSpriteFrameId = gsl::narrow<int16_t>((modelType->meshBaseIndex + i).index);
    setParent(particle, object.m_state.location.room->node);
    object.getWorld().getObjectManager().registerParticle(std::move(particle));

    BOOST_LOG_TRIVIAL(trace) << "Shatter model: mesh " << i << " converted";
  }

  for(size_t i = 0; i < object.getSkeleton()->getBoneCount(); ++i)
  {
    if(object.getSkeleton()->isVisible(i))
      return false;
  }

  return true;
}
} // namespace engine::objects
