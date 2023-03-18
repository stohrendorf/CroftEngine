#include "shatter.h"

#include "core/containeroffset.h"
#include "core/id.h"
#include "core/vec.h"
#include "engine/engine.h"
#include "engine/engineconfig.h"
#include "engine/items_tr1.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/particle.h"
#include "engine/presenter.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/rendermeshdata.h"
#include "engine/world/room.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "modelobject.h"
#include "objectstate.h"
#include "render/rendersettings.h"
#include "render/scene/mesh.h" // IWYU pragma: keep
#include "render/scene/node.h"

#include <boost/log/trivial.hpp>
#include <cstddef>
#include <cstdint>
#include <gl/pixel.h>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

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
  gsl_Assert(modelType != nullptr);
  gsl_Assert(modelType->bones.size() == object.getSkeleton()->getBoneCount());
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
    compositor.append(*modelType->bones[i].mesh, gl::SRGBA8{0, 0, 0, 0});
    auto particle = gsl::make_shared<MeshShrapnelParticle>(
      Location{object.m_state.location.room, core::TRVec{object.getSkeleton()->getMeshPartTranslationWorld(i)}},
      object.getWorld(),
      compositor.toMesh(
        *object.getWorld().getPresenter().getMaterialManager(),
        false,
        true,
        [&settings = object.getWorld().getEngine().getEngineConfig()]()
        {
          return settings->animSmoothing;
        },
        [&settings = object.getWorld().getEngine().getEngineConfig()->renderSettings]()
        {
          return !settings.lightingModeActive ? 0 : settings.lightingMode;
        },
        "shatter-part:" + std::to_string(i)),
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
