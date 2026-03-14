#include "particle.h"

#include "audioengine.h"
#include "core/angle.h"
#include "core/boundingbox.h"
#include "core/genericvec.h"
#include "core/interval.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine.h"
#include "engineconfig.h"
#include "heightinfo.h"
#include "items_tr1.h"
#include "location.h"
#include "objectmanager.h"
#include "objects/laraobject.h"
#include "objects/objectstate.h"
#include "particlecollection.h"
#include "presenter.h"
#include "qs/qs.h"
#include "render/material/spritematerialmode.h"
#include "render/rendersettings.h"
#include "render/scene/mesh.h" // IWYU pragma: keep
#include "render/scene/node.h"
#include "skeletalmodelnode.h"
#include "soundeffects_tr1.h"
#include "util/helpers.h"
#include "world/rendermeshdata.h"
#include "world/room.h"
#include "world/skeletalmodeltype.h"
#include "world/sprite.h"
#include "world/world.h"

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>
#include <cstddef>
#include <cstdint>
#include <gl/pixel.h>
#include <gl/renderstate.h>
#include <gl/vertexbuffer.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <gsl-lite/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

namespace engine
{
void Particle::initRenderables(world::World& world, const render::material::SpriteMaterialMode mode)
{
  if(const auto& modelType = world.getWorldGeometry().findAnimatedModelForType(object_number))
  {
    for(const auto& bone : modelType->bones)
    {
      world::RenderMeshDataCompositor compositor;
      compositor.append(*bone.mesh, gl::SRGBA8{0, 0, 0, 0});
      m_meshes.emplace_back(compositor.toMesh(
                              world.getEngine().getPresenter().getRenderSystem().getMaterialManager(),
                              false,
                              false,
                              [&world]() -> bool
                              {
                                return world.getEngine().getEngineConfig()->animSmoothing;
                              },
                              [&world]
                              {
                                const auto& settings = world.getEngine().getEngineConfig()->renderSettings;
                                return !settings.lightingModeActive ? 0 : settings.lightingMode;
                              },
                              "particle"),
                            nullptr);
    }
  }
  else if(const auto& spriteSequence = world.getWorldGeometry().findSpriteSequenceForType(object_number))
  {
    for(const world::Sprite& spr : spriteSequence->sprites)
    {
      switch(mode)
      {
      case render::material::SpriteMaterialMode::YAxisBound:
        m_meshes.emplace_back(spr.yBoundMesh, nullptr);
        break;
      case render::material::SpriteMaterialMode::Billboard:
        m_meshes.emplace_back(spr.billboardMesh, nullptr);
        break;
      case render::material::SpriteMaterialMode::InstancedBillboard:
        m_meshes.emplace_back(spr.instancedBillboardMesh);
        break;
      }
    }
  }
  else
  {
    BOOST_LOG_TRIVIAL(warning) << "Missing sprite/model referenced by particle: "
                               << toString(static_cast<TR1ItemId>(object_number.get()));
    return;
  }

  if(!m_meshes.empty())
  {
    setRenderable(std::get<0>(m_meshes.front()));
    m_lighting.bind(*this, world);
  }
}

glm::vec3 Particle::getPosition() const
{
  return location.position.toRenderSystem();
}

Particle::Particle(const std::string& id,
                   const core::TypeId& objectNumber,
                   const gsl_lite::not_null<const world::Room*>& room,
                   world::World& world,
                   const render::material::SpriteMaterialMode mode,
                   const bool withoutParent,
                   const std::shared_ptr<render::scene::Mesh>& renderable)
    : Node{id}
    , Emitter{gsl_lite::not_null{world.getEngine().getPresenter().getSoundEngine().get().get()}}
    , location{room}
    , object_number{objectNumber}
    , m_withoutParent{withoutParent}
{
  if(renderable == nullptr)
  {
    initRenderables(world, mode);
  }
  else
  {
    m_meshes.emplace_back(gsl_lite::not_null{renderable}, nullptr);
    setRenderable(std::get<0>(m_meshes.front()));
    m_lighting.bind(*this, world);
  }
}

Particle::Particle(const std::string& id,
                   const core::TypeId& objectNumber,
                   Location location,
                   world::World& world,
                   const render::material::SpriteMaterialMode mode,
                   const bool withoutParent,
                   const std::shared_ptr<render::scene::Mesh>& renderable)
    : Node{id}
    , Emitter{gsl_lite::not_null{world.getEngine().getPresenter().getSoundEngine().get().get()}}
    , location{std::move(location)}
    , object_number{objectNumber}
    , m_withoutParent{withoutParent}
{
  if(renderable == nullptr)
  {
    initRenderables(world, mode);
  }
  else
  {
    m_meshes.emplace_back(renderable, nullptr);
    setRenderable(std::get<0>(m_meshes.front()));
    m_lighting.bind(*this, world);
  }
}

void Particle::applyLogicTransform()
{
  location.updateRoom();

  predictedPosition = location.position;
  predictedAngle = angle;

  if(speed != 0_spd)
  {
    predictedPosition += util::pitch(speed * 1_frame, angle.Y);
  }
  if(fall_speed != 0_spd)
  {
    predictedPosition.Y += fall_speed * 1_frame;
  }

  interpolateTransform(0);
}

void Particle::interpolateTransform(const float interTickFactor)
{
  m_lighting.update(m_shade.value_or(core::Shade{core::Shade::type{-1}}), *location.room);

  const auto pos = core::lerp(location.position, predictedPosition, interTickFactor);
  const auto rot = core::lerp(angle, predictedAngle, interTickFactor);

  auto l = pos.toRenderSystem();
  if(!m_withoutParent)
    l -= location.room->position.toRenderSystem();

  auto transform = glm::scale(rot.toMatrix(), glm::vec3{scale});
  transform[3] = glm::vec4{l, 1.0f};
  setLocalMatrix(transform);
}

std::tuple<std::shared_ptr<render::scene::Mesh>, std::shared_ptr<gl::VertexBuffer<glm::mat4>>>
  Particle::getCurrentMesh() const
{
  if(m_meshes.empty())
    return {nullptr, nullptr};

  return m_meshes.front();
}

void Particle::nextFrame()
{
  --negSpriteFrameId;

  if(m_meshes.empty())
    return;

  m_meshes.emplace_back(m_meshes.front());
  m_meshes.pop_front();
  setRenderable(std::get<0>(m_meshes.front()));
}

BloodSplatterParticle::BloodSplatterParticle(const Location& location,
                                             const core::Speed& speed_,
                                             const core::Angle& angle_,
                                             world::World& world)
    : Particle{"blood-splat", TR1ItemId::Blood, location, world, render::material::SpriteMaterialMode::Billboard}
{
  speed = speed_;
  angle.Y = angle_;
}

bool BloodSplatterParticle::updateLogic(world::World& world)
{
  location.position += util::pitch(speed * 1_frame, angle.Y);
  ++timePerSpriteFrame;
  if(timePerSpriteFrame != 4)
  {
    applyLogicTransform();
    return true;
  }

  timePerSpriteFrame = 0;
  nextFrame();
  if(gsl_lite::narrow<size_t>(-negSpriteFrameId)
     >= world.getWorldGeometry().findSpriteSequenceForType(object_number)->sprites.size())
    return false;

  applyLogicTransform();
  return true;
}

bool SplashParticle::updateLogic(world::World& world)
{
  nextFrame();

  if(gsl_lite::narrow<size_t>(-negSpriteFrameId)
     >= world.getWorldGeometry().findSpriteSequenceForType(object_number)->sprites.size())
  {
    return false;
  }

  location.position += util::pitch(speed * 1_frame, angle.Y);

  applyLogicTransform();
  return true;
}

SplashParticle::SplashParticle(const Location& location, world::World& world, const bool waterfall)
    : Particle{"splash", TR1ItemId::Splash, location, world, render::material::SpriteMaterialMode::YAxisBound}
{
  if(!waterfall)
  {
    speed = util::rand15(128_spd);
    angle.Y = core::auToAngle(int16_t{2} * util::rand15s());
  }
  else
  {
    this->location.position.X += util::rand15s(1_sectors);
    this->location.position.Z += util::rand15s(1_sectors);
  }
  getRenderState().setScissorTest(false);
}

BubbleParticle::BubbleParticle(const Location& location,
                               world::World& world,
                               const bool onlyInWater,
                               const bool instanced)
    : Particle{"bubble",
               TR1ItemId::Bubbles,
               location,
               world,
               instanced ? render::material::SpriteMaterialMode::InstancedBillboard
                         : render::material::SpriteMaterialMode::Billboard,
               instanced}
    , m_onlyInWater{onlyInWater}
{
  speed = 10_spd + util::rand15(6_spd);

  const int n = util::rand15(3);
  for(int i = 0; i < n; ++i)
    nextFrame();
}

bool BubbleParticle::updateLogic(world::World& world)
{
  angle.X += 13_deg;
  angle.Y += 9_deg;
  location.position += util::pitch(circleRadius, angle.Y, -speed * 1_frame);
  const auto sector = location.updateRoom();
  if(m_onlyInWater && !location.room->isWaterRoom)
  {
    return false;
  }

  if(const auto ceiling = HeightInfo::fromCeiling(sector, location.position, world.getObjectManager().getObjects()).y;
     ceiling == core::InvalidHeight || location.position.Y <= ceiling)
  {
    return false;
  }

  applyLogicTransform();
  return true;
}

FlameParticle::FlameParticle(const Location& location, world::World& world, const bool randomize)
    : Particle{"flame", TR1ItemId::Flame, location, world, render::material::SpriteMaterialMode::YAxisBound}
{
  timePerSpriteFrame = 0;
  negSpriteFrameId = 0;
  setShade(core::Shade{core::Shade::type{4096}});

  if(randomize)
  {
    timePerSpriteFrame
      = -util::rand15(gsl_lite::narrow_cast<int16_t>(world.getObjectManager().getLara().getSkeleton()->getBoneCount()))
        - int16_t{1};
    for(auto n = util::rand15(getLength()); n != 0; --n)
      nextFrame();
  }
}

bool FlameParticle::updateLogic(world::World& world)
{
  nextFrame();
  const auto& spriteSequence = world.getWorldGeometry().findSpriteSequenceForType(object_number);
  if(spriteSequence == nullptr)
    return false;

  if(gsl_lite::narrow<size_t>(-negSpriteFrameId) >= spriteSequence->sprites.size())
    negSpriteFrameId = 0;

  const auto& lara = world.getObjectManager().getLara();
  if(timePerSpriteFrame >= 0)
  {
    // not attached to lara
    world.getAudioEngine().playSoundEffect(TR1SoundEffect::Burning, this);
    if(timePerSpriteFrame != 0)
    {
      --timePerSpriteFrame;
      applyLogicTransform();
      return true;
    }

    BOOST_ASSERT(timePerSpriteFrame == 0);
    if(lara.isNearInexact(location.position, 600_len))
    {
      // lara is close enough to be hurt by heat
      world.hitLara(3_hp);

      const auto& laraState = lara.m_state;
      const auto distSq = util::square(laraState.location.position.X - location.position.X)
                          + util::square(laraState.location.position.Z - location.position.Z);
      if(distSq < util::square(300_len))
      {
        // only attach a new flame to lara every 100 frames
        timePerSpriteFrame = 100;

        const auto alreadyAttachedToLara = std::ranges::any_of(world.getObjectManager().getParticles(),
                                                               [](const gslu::nn_shared<Particle>& particle)
                                                               {
                                                                 return particle->object_number == TR1ItemId::Flame
                                                                        && particle->timePerSpriteFrame == -1;
                                                               });

        if(!alreadyAttachedToLara)
        {
          const auto particle = gsl_lite::make_shared<FlameParticle>(location, world);
          particle->timePerSpriteFrame = -1;
          if(!withoutParent())
            setParent(particle, location.room->node);
          world.getObjectManager().registerParticle(particle);
        }
      }
    }
  }
  else
  {
    // this flame is attached to lara
    const auto itemSpheres = lara.getSkeleton()->getBoneCollisionSpheres();
    location.position = core::TRVec{
      itemSpheres.at(gsl_lite::narrow_cast<int>(-timePerSpriteFrame) - 1)
        .relative(core::TRVec{0_len, timePerSpriteFrame == -1 ? -100_len : 0_len, 0_len}.toRenderSystem())};

    if(const auto waterHeight = world::getWaterSurfaceHeight(location);
       !waterHeight.has_value() || *waterHeight >= location.position.Y)
    {
      world.getAudioEngine().playSoundEffect(TR1SoundEffect::Burning, this);
      world.hitLara(3_hp);
    }
    else
    {
      timePerSpriteFrame = 0;
      world.getAudioEngine().stopSoundEffect(TR1SoundEffect::Burning, this);
      return false;
    }
  }

  applyLogicTransform();
  return true;
}

bool MeshShrapnelParticle::updateLogic(world::World& world)
{
  angle.X += 5_deg;
  angle.Z += 10_deg;
  fall_speed += core::Gravity * 1_frame;

  location.position += util::pitch(speed * 1_frame, angle.Y, fall_speed * 1_frame);

  const auto sector = location.updateRoom();
  if(const auto ceiling = HeightInfo::fromCeiling(sector, location.position, world.getObjectManager().getObjects()).y;
     ceiling > location.position.Y)
  {
    location.position.Y = ceiling;
    fall_speed = -fall_speed;
  }

  const auto floor = HeightInfo::fromFloor(sector, location.position, world.getObjectManager().getObjects()).y;

  bool explode = false;

  if(floor <= location.position.Y)
  {
    if(m_damageRadius <= 0_len)
      return false;

    explode = true;
  }
  else if(auto& lara = world.getObjectManager().getLara(); lara.isNearInexact(location.position, 2 * m_damageRadius))
  {
    lara.m_state.is_hit = true;
    if(m_damageRadius <= 0_len)
      return false;

    world.hitLara(m_damageRadius * 1_hp / 1_len);
    explode = true;

    lara.forceSourcePosition = &location.position;
    lara.explosionStumblingDuration = 5_frame;
  }

  if(!withoutParent())
    setParent(this, location.room->node);
  applyLogicTransform();

  if(!explode)
    return true;

  const auto particle = gsl_lite::make_shared<ExplosionParticle>(location, world, fall_speed, angle);
  if(!withoutParent())
    setParent(particle, location.room->node);
  world.getObjectManager().registerParticle(particle);
  world.getAudioEngine().playSoundEffect(TR1SoundEffect::ShrapnelExplosion, particle.get().get());
  return false;
}

MeshShrapnelParticle::MeshShrapnelParticle(const Location& location,
                                           world::World& world,
                                           const gslu::nn_shared<render::scene::Mesh>& renderable,
                                           const bool torsoBoss,
                                           const core::Length& damageRadius)
    : Particle{"meshShrapnel",
               TR1ItemId::MeshShrapnel,
               location,
               world,
               render::material::SpriteMaterialMode::YAxisBound,
               false,
               renderable}
    , m_damageRadius{damageRadius}
{
  clearMeshes();

  angle.Y = core::Angle{util::rand15s() * 2};
  speed = util::rand15(256_spd);
  fall_speed = util::rand15(-256_spd);
  if(!torsoBoss)
  {
    speed /= 2;
    fall_speed /= 2;
  }
}

MutantAmmoParticle::MutantAmmoParticle(const Location& location, world::World& world, const TR1ItemId itemType)
    : Particle{"mutantAmmo", itemType, location, world, render::material::SpriteMaterialMode::YAxisBound}
{
}

void MutantAmmoParticle::aimLaraChest(world::World& world)
{
  const auto d = world.getObjectManager().getLara().m_state.location.position - location.position;
  const auto bbox = world.getObjectManager().getLara().getSkeleton()->getBoundingBox();
  angle.X = util::rand15s(256_au)
            - angleFromAtan(bbox.y.max - bbox.y.size() * 3 / 4 + d.Y, sqrt(util::square(d.X) + util::square(d.Z)));
  angle.Y = util::rand15s(256_au) + angleFromAtan(d.X, d.Z);
}

bool MutantBulletParticle::updateLogic(world::World& world)
{
  location.position += util::yawPitch(speed * 1_frame, angle);
  const auto sector = location.updateRoom();
  if(!withoutParent())
    setParent(this, location.room->node);
  if(HeightInfo::fromFloor(sector, location.position, world.getObjectManager().getObjects()).y <= location.position.Y
     || HeightInfo::fromCeiling(sector, location.position, world.getObjectManager().getObjects()).y
          >= location.position.Y)
  {
    const auto particle = gsl_lite::make_shared<RicochetParticle>(location, world);
    particle->timePerSpriteFrame = 6;
    if(!withoutParent())
      setParent(particle, location.room->node);
    world.getObjectManager().registerParticle(particle);
    world.getAudioEngine().playSoundEffect(TR1SoundEffect::Ricochet, particle.get().get());
    return false;
  }
  else if(world.getObjectManager().getLara().isNearInexact(location.position, 200_len))
  {
    world.hitLara(30_hp);
    const auto& laraState = world.getObjectManager().getLara().m_state;
    const auto particle = gsl_lite::make_shared<BloodSplatterParticle>(location, speed, angle.Y, world);
    if(!withoutParent())
      setParent(particle, location.room->node);
    world.getObjectManager().registerParticle(particle);
    world.getAudioEngine().playSoundEffect(TR1SoundEffect::BulletHitsLara, particle.get().get());
    angle.Y = laraState.rotation.Y;
    speed = laraState.speed;
    timePerSpriteFrame = 0;
    negSpriteFrameId = 0;
    return false;
  }

  applyLogicTransform();
  return true;
}

bool MutantGrenadeParticle::updateLogic(world::World& world)
{
  location.position += util::yawPitch(speed * 1_frame, angle);
  const auto sector = location.updateRoom();
  if(!withoutParent())
    setParent(this, location.room->node);
  if(HeightInfo::fromFloor(sector, location.position, world.getObjectManager().getObjects()).y <= location.position.Y
     || HeightInfo::fromCeiling(sector, location.position, world.getObjectManager().getObjects()).y
          >= location.position.Y)
  {
    const auto particle = gsl_lite::make_shared<ExplosionParticle>(location, world, fall_speed, angle);
    if(!withoutParent())
      setParent(particle, location.room->node);
    world.getObjectManager().registerParticle(particle);
    world.getAudioEngine().playSoundEffect(TR1SoundEffect::ShrapnelExplosion, particle.get().get());

    const auto dd = location.position - world.getObjectManager().getLara().m_state.location.position;
    if(const auto d = util::square(dd.X) + util::square(dd.Y) + util::square(dd.Z); d < util::square(1024_len))
    {
      world.hitLara(100_hp * (util::square(1_sectors) - d) / util::square(1_sectors));
    }

    return false;
  }
  else if(world.getObjectManager().getLara().isNearInexact(location.position, 200_len))
  {
    world.hitLara(100_hp);
    const auto particle = gsl_lite::make_shared<ExplosionParticle>(location, world, fall_speed, angle);
    if(!withoutParent())
      setParent(particle, location.room->node);
    world.getObjectManager().registerParticle(particle);
    world.getAudioEngine().playSoundEffect(TR1SoundEffect::ShrapnelExplosion, particle.get().get());

    if(!world.getObjectManager().getLara().isDead())
    {
      world.getObjectManager().getLara().playSoundEffect(TR1SoundEffect::LaraHurt);
      world.getObjectManager().getLara().forceSourcePosition = &particle->location.position;
      world.getObjectManager().getLara().explosionStumblingDuration = 5_frame;
    }

    angle.Y = world.getObjectManager().getLara().m_state.rotation.Y;
    speed = world.getObjectManager().getLara().m_state.speed;
    timePerSpriteFrame = 0;
    negSpriteFrameId = 0;
    return false;
  }

  applyLogicTransform();
  return true;
}

LavaParticle::LavaParticle(const Location& location, world::World& world)
    : Particle{"lava", TR1ItemId::LavaParticles, location, world, render::material::SpriteMaterialMode::Billboard}
{
  angle.Y = util::rand15(180_deg) * 2;
  speed = util::rand15(32_spd);
  fall_speed = -util::rand15(165_spd);
  negSpriteFrameId = util::rand15(int16_t{-4});
}

bool LavaParticle::updateLogic(world::World& world)
{
  fall_speed += core::Gravity * 1_frame;
  location.position += util::pitch(speed * 1_frame, angle.Y, fall_speed * 1_frame);

  const auto sector = location.updateRoom();
  if(!withoutParent())
    setParent(this, location.room->node);
  if(HeightInfo::fromFloor(sector, location.position, world.getObjectManager().getObjects()).y <= location.position.Y
     || HeightInfo::fromCeiling(sector, location.position, world.getObjectManager().getObjects()).y
          > location.position.Y)
  {
    return false;
  }

  if(world.getObjectManager().getLara().isNearInexact(location.position, 200_len))
  {
    world.hitLara(10_hp);
    return false;
  }

  applyLogicTransform();
  return true;
}

SparkleParticle::SparkleParticle(const Location& location, world::World& world)
    : Particle{"sparkles", TR1ItemId::Sparkles, location, world, render::material::SpriteMaterialMode::Billboard}
{
}

bool SparkleParticle::updateLogic(world::World&)
{
  applyLogicTransform();

  ++timePerSpriteFrame;
  if(timePerSpriteFrame != 1)
    return true;

  --negSpriteFrameId;
  timePerSpriteFrame = 0;
  return gsl_lite::narrow<size_t>(-negSpriteFrameId) < getLength();
}

MuzzleFlashParticle::MuzzleFlashParticle(const Location& location, world::World& world, const core::Angle& yAngle)
    : Particle{
        "muzzle-flash", TR1ItemId::MuzzleFlash, location, world, render::material::SpriteMaterialMode::YAxisBound}
{
  angle.Y = yAngle;
  timePerSpriteFrame = 3;
  setShade(core::Shade{core::Shade::type{4096}});
}

bool MuzzleFlashParticle::updateLogic(world::World&)
{
  --timePerSpriteFrame;
  if(timePerSpriteFrame == 0)
    return false;

  angle.Z = util::rand15s(+180_deg);
  applyLogicTransform();
  return true;
}

ExplosionParticle::ExplosionParticle(const Location& location,
                                     world::World& world,
                                     const core::Speed& fallSpeed,
                                     const core::TRRotation& angle)
    : Particle{"explosion", TR1ItemId::Explosion, location, world, render::material::SpriteMaterialMode::Billboard}
{
  fall_speed = fallSpeed;
  this->angle = angle;
}

bool ExplosionParticle::updateLogic(world::World&)
{
  ++timePerSpriteFrame;
  if(timePerSpriteFrame == 2)
  {
    timePerSpriteFrame = 0;
    nextFrame();
    if(static_cast<size_t>(-negSpriteFrameId) >= getLength())
    {
      return false;
    }
  }

  applyLogicTransform();
  return true;
}

SmokeParticle::SmokeParticle(const Location& location, world::World& world, const core::TRRotation& rotation)
    : Particle{"smoke", TR1ItemId::Smoke, location, world, render::material::SpriteMaterialMode::YAxisBound}
{
  angle = rotation;
}

bool SmokeParticle::updateLogic(world::World&)
{
  applyLogicTransform();

  ++timePerSpriteFrame;
  if(timePerSpriteFrame < 3)
    return true;

  timePerSpriteFrame = 0;
  nextFrame();
  return gsl_lite::narrow<size_t>(-negSpriteFrameId) < getLength();
}

RicochetParticle::RicochetParticle(const Location& location, world::World& world)
    : Particle{"ricochet", TR1ItemId::Ricochet, location, world, render::material::SpriteMaterialMode::YAxisBound}
{
  timePerSpriteFrame = 4;

  const int n = util::rand15(3);
  for(int i = 0; i < n; ++i)
    nextFrame();
}

bool RicochetParticle::updateLogic(world::World&)
{
  applyLogicTransform();

  --timePerSpriteFrame;
  return timePerSpriteFrame != 0;
}

gslu::nn_shared<Particle> createMuzzleFlash(world::World& world,
                                            const Location& location,
                                            const core::Speed& /*speed*/,
                                            const core::Angle& angle)
{
  auto particle = gsl_lite::make_shared<MuzzleFlashParticle>(location, world, angle);
  setParent(particle, location.room->node);
  return particle;
}

gslu::nn_shared<Particle> createMutantBullet(world::World& world,
                                             const Location& location,
                                             const core::Speed& /*speed*/,
                                             const core::Angle& angle)
{
  auto particle = gsl_lite::make_shared<MutantBulletParticle>(location, world, angle);
  setParent(particle, location.room->node);
  return particle;
}

gslu::nn_shared<Particle> createMutantGrenade(world::World& world,
                                              const Location& location,
                                              const core::Speed& /*speed*/,
                                              const core::Angle& angle)
{
  auto particle = gsl_lite::make_shared<MutantGrenadeParticle>(location, world, angle);
  setParent(particle, location.room->node);
  return particle;
}

gslu::nn_shared<Particle>
  createBloodSplat(world::World& world, const Location& location, const core::Speed& speed, const core::Angle& angle)
{
  auto particle = gsl_lite::make_shared<BloodSplatterParticle>(location, speed, angle, world);
  setParent(particle, location.room->node);
  return particle;
}
} // namespace engine