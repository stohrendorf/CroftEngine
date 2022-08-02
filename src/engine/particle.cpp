#include "particle.h"

#include "audioengine.h"
#include "core/boundingbox.h"
#include "core/genericvec.h"
#include "core/interval.h"
#include "core/magic.h"
#include "core/vec.h"
#include "heightinfo.h"
#include "items_tr1.h"
#include "location.h"
#include "objectmanager.h"
#include "objects/laraobject.h"
#include "objects/objectstate.h"
#include "presenter.h"
#include "render/scene/mesh.h" // IWYU pragma: keep
#include "render/scene/spritematerialmode.h"
#include "skeletalmodelnode.h"
#include "soundeffects_tr1.h"
#include "world/rendermeshdata.h"
#include "world/room.h"
#include "world/skeletalmodeltype.h"
#include "world/sprite.h"
#include "world/world.h"

#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>
#include <gl/renderstate.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <gslu.h>
#include <type_traits>
#include <utility>
#include <vector>

namespace render::scene
{
class Renderable;
}

namespace engine
{
void Particle::initRenderables(world::World& world, render::scene::SpriteMaterialMode mode)
{
  if(const auto& modelType = world.findAnimatedModelForType(object_number))
  {
    for(const auto& bone : modelType->bones)
    {
      world::RenderMeshDataCompositor compositor;
      compositor.append(*bone.mesh, gl::SRGBA8{0, 0, 0, 0});
      m_meshes.emplace_back(compositor.toMesh(*world.getPresenter().getMaterialManager(), false, false, {}), nullptr);
    }
  }
  else if(const auto& spriteSequence = world.findSpriteSequenceForType(object_number))
  {
    for(const world::Sprite& spr : spriteSequence->sprites)
    {
      switch(mode)
      {
      case render::scene::SpriteMaterialMode::YAxisBound:
        m_meshes.emplace_back(spr.yBoundMesh, nullptr);
        break;
      case render::scene::SpriteMaterialMode::Billboard:
        m_meshes.emplace_back(spr.billboardMesh, nullptr);
        break;
      case render::scene::SpriteMaterialMode::InstancedBillboard:
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
    m_lighting.bind(*this);
  }
}

glm::vec3 Particle::getPosition() const
{
  return location.position.toRenderSystem();
}

Particle::Particle(const std::string& id,
                   const core::TypeId& objectNumber,
                   const gsl::not_null<const world::Room*>& room,
                   world::World& world,
                   render::scene::SpriteMaterialMode mode,
                   bool withoutParent,
                   const std::shared_ptr<render::scene::Mesh>& renderable)
    : Node{id}
    , Emitter{gsl::not_null{world.getPresenter().getSoundEngine().get()}}
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
    m_meshes.emplace_back(gsl::not_null{renderable}, nullptr);
    setRenderable(std::get<0>(m_meshes.front()));
    m_lighting.bind(*this);
  }
}

Particle::Particle(const std::string& id,
                   const core::TypeId& objectNumber,
                   Location location,
                   world::World& world,
                   render::scene::SpriteMaterialMode mode,
                   bool withoutParent,
                   const std::shared_ptr<render::scene::Mesh>& renderable)
    : Node{id}
    , Emitter{gsl::not_null{world.getPresenter().getSoundEngine().get()}}
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
    m_lighting.bind(*this);
  }
}

void Particle::applyTransform()
{
  location.updateRoom();
  m_lighting.update(m_shade.value_or(core::Shade{core::Shade::type{-1}}), *location.room);

  auto l = location.position.toRenderSystem();
  if(!m_withoutParent)
    l -= location.room->position.toRenderSystem();

  auto transform = glm::scale(angle.toMatrix(), glm::vec3{scale});
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
    : Particle{"bloodsplat", TR1ItemId::Blood, location, world, render::scene::SpriteMaterialMode::Billboard}
{
  speed = speed_;
  angle.Y = angle_;
}

bool BloodSplatterParticle::update(world::World& world)
{
  location.position += util::pitch(speed * 1_frame, angle.Y);
  ++timePerSpriteFrame;
  if(timePerSpriteFrame != 4)
  {
    applyTransform();
    return true;
  }

  timePerSpriteFrame = 0;
  nextFrame();
  if(gsl::narrow<size_t>(-negSpriteFrameId) >= world.findSpriteSequenceForType(object_number)->sprites.size())
    return false;

  applyTransform();
  return true;
}

bool SplashParticle::update(world::World& world)
{
  nextFrame();

  if(gsl::narrow<size_t>(-negSpriteFrameId) >= world.findSpriteSequenceForType(object_number)->sprites.size())
  {
    return false;
  }

  location.position += util::pitch(speed * 1_frame, angle.Y);

  applyTransform();
  return true;
}

SplashParticle::SplashParticle(const Location& location, world::World& world, const bool waterfall)
    : Particle{"splash", TR1ItemId::Splash, location, world, render::scene::SpriteMaterialMode::YAxisBound}
{
  if(!waterfall)
  {
    speed = util::rand15(128_spd);
    angle.Y = core::auToAngle(2 * util::rand15s());
  }
  else
  {
    this->location.position.X += util::rand15s(1_sectors);
    this->location.position.Z += util::rand15s(1_sectors);
  }
  getRenderState().setScissorTest(false);
}

BubbleParticle::BubbleParticle(const Location& location, world::World& world, bool onlyInWater, bool instanced)
    : Particle{"bubble",
               TR1ItemId::Bubbles,
               location,
               world,
               instanced ? render::scene::SpriteMaterialMode::InstancedBillboard
                         : render::scene::SpriteMaterialMode::Billboard,
               instanced}
    , m_onlyInWater{onlyInWater}
{
  speed = 10_spd + util::rand15(6_spd);

  const int n = util::rand15(3);
  for(int i = 0; i < n; ++i)
    nextFrame();
}

bool BubbleParticle::update(world::World& world)
{
  angle.X += 13_deg;
  angle.Y += 9_deg;
  location.position += util::pitch(circleRadius, angle.Y, -speed * 1_frame);
  auto sector = location.updateRoom();
  if(m_onlyInWater && !location.room->isWaterRoom)
  {
    return false;
  }

  if(const auto ceiling = HeightInfo::fromCeiling(sector, location.position, world.getObjectManager().getObjects()).y;
     ceiling == core::InvalidHeight || location.position.Y <= ceiling)
  {
    return false;
  }

  applyTransform();
  return true;
}

FlameParticle::FlameParticle(const Location& location, world::World& world, bool randomize)
    : Particle{"flame", TR1ItemId::Flame, location, world, render::scene::SpriteMaterialMode::YAxisBound}
{
  timePerSpriteFrame = 0;
  negSpriteFrameId = 0;
  setShade(core::Shade{core::Shade::type{4096}});

  if(randomize)
  {
    timePerSpriteFrame = -int(util::rand15(world.getObjectManager().getLara().getSkeleton()->getBoneCount())) - 1;
    for(auto n = util::rand15(getLength()); n != 0; --n)
      nextFrame();
  }
}

bool FlameParticle::update(world::World& world)
{
  nextFrame();
  if(gsl::narrow<size_t>(-negSpriteFrameId) >= world.findSpriteSequenceForType(object_number)->sprites.size())
    negSpriteFrameId = 0;

  objects::LaraObject& lara = world.getObjectManager().getLara();
  if(timePerSpriteFrame >= 0)
  {
    // not attached to lara
    world.getAudioEngine().playSoundEffect(TR1SoundEffect::Burning, this);
    if(timePerSpriteFrame != 0)
    {
      --timePerSpriteFrame;
      applyTransform();
      return true;
    }

    BOOST_ASSERT(timePerSpriteFrame == 0);
    if(lara.isNearInexact(location.position, 600_len))
    {
      // lara is close enough to be hurt by heat
      auto& laraState = lara.m_state;
      laraState.health -= 3_hp;
      laraState.is_hit = true;

      const auto distSq = util::square(laraState.location.position.X - location.position.X)
                          + util::square(laraState.location.position.Z - location.position.Z);
      if(distSq < util::square(300_len))
      {
        // only attach a new flame to lara every 100 frames
        timePerSpriteFrame = 100;

        const auto alreadyAttachedToLara
          = std::any_of(world.getObjectManager().getParticles().begin(),
                        world.getObjectManager().getParticles().end(),
                        [](const gslu::nn_shared<Particle>& particle)
                        {
                          return particle->object_number == TR1ItemId::Flame && particle->timePerSpriteFrame == -1;
                        });

        if(!alreadyAttachedToLara)
        {
          const auto particle = gsl::make_shared<FlameParticle>(location, world);
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
      itemSpheres.at(-timePerSpriteFrame - 1)
        .relative(core::TRVec{0_len, timePerSpriteFrame == -1 ? -100_len : 0_len, 0_len}.toRenderSystem())};

    if(const auto waterHeight = world::getWaterSurfaceHeight(location);
       !waterHeight.has_value() || *waterHeight >= location.position.Y)
    {
      world.getAudioEngine().playSoundEffect(TR1SoundEffect::Burning, this);
      lara.m_state.health -= 3_hp;
      lara.m_state.is_hit = true;
    }
    else
    {
      timePerSpriteFrame = 0;
      world.getAudioEngine().stopSoundEffect(TR1SoundEffect::Burning, this);
      return false;
    }
  }

  applyTransform();
  return true;
}

bool MeshShrapnelParticle::update(world::World& world)
{
  angle.X += 5_deg;
  angle.Z += 10_deg;
  fall_speed += core::Gravity * 1_frame;

  location.position += util::pitch(speed * 1_frame, angle.Y, fall_speed * 1_frame);

  const auto sector = location.updateRoom();
  const auto ceiling = HeightInfo::fromCeiling(sector, location.position, world.getObjectManager().getObjects()).y;
  if(ceiling > location.position.Y)
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

    lara.m_state.health -= m_damageRadius * 1_hp / 1_len;
    explode = true;

    lara.forceSourcePosition = &location.position;
    lara.explosionStumblingDuration = 5_frame;
  }

  if(!withoutParent())
    setParent(this, location.room->node);
  applyTransform();

  if(!explode)
    return true;

  const auto particle = gsl::make_shared<ExplosionParticle>(location, world, fall_speed, angle);
  if(!withoutParent())
    setParent(particle, location.room->node);
  world.getObjectManager().registerParticle(particle);
  world.getAudioEngine().playSoundEffect(TR1SoundEffect::Explosion2, particle.get().get());
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
               render::scene::SpriteMaterialMode::YAxisBound,
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
    : Particle{"mutantAmmo", itemType, location, world, render::scene::SpriteMaterialMode::YAxisBound}
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

bool MutantBulletParticle::update(world::World& world)
{
  location.position += util::yawPitch(speed * 1_frame, angle);
  const auto sector = location.updateRoom();
  if(!withoutParent())
    setParent(this, location.room->node);
  if(HeightInfo::fromFloor(sector, location.position, world.getObjectManager().getObjects()).y <= location.position.Y
     || HeightInfo::fromCeiling(sector, location.position, world.getObjectManager().getObjects()).y
          >= location.position.Y)
  {
    auto particle = gsl::make_shared<RicochetParticle>(location, world);
    particle->timePerSpriteFrame = 6;
    if(!withoutParent())
      setParent(particle, location.room->node);
    world.getObjectManager().registerParticle(particle);
    world.getAudioEngine().playSoundEffect(TR1SoundEffect::Ricochet, particle.get().get());
    return false;
  }
  else if(world.getObjectManager().getLara().isNearInexact(location.position, 200_len))
  {
    auto& laraState = world.getObjectManager().getLara().m_state;
    laraState.health -= 30_hp;
    auto particle = gsl::make_shared<BloodSplatterParticle>(location, speed, angle.Y, world);
    if(!withoutParent())
      setParent(particle, location.room->node);
    world.getObjectManager().registerParticle(particle);
    world.getAudioEngine().playSoundEffect(TR1SoundEffect::BulletHitsLara, particle.get().get());
    laraState.is_hit = true;
    angle.Y = laraState.rotation.Y;
    speed = laraState.speed;
    timePerSpriteFrame = 0;
    negSpriteFrameId = 0;
    return false;
  }

  applyTransform();
  return true;
}

bool MutantGrenadeParticle::update(world::World& world)
{
  location.position += util::yawPitch(speed * 1_frame, angle);
  const auto sector = location.updateRoom();
  if(!withoutParent())
    setParent(this, location.room->node);
  if(HeightInfo::fromFloor(sector, location.position, world.getObjectManager().getObjects()).y <= location.position.Y
     || HeightInfo::fromCeiling(sector, location.position, world.getObjectManager().getObjects()).y
          >= location.position.Y)
  {
    auto particle = gsl::make_shared<ExplosionParticle>(location, world, fall_speed, angle);
    if(!withoutParent())
      setParent(particle, location.room->node);
    world.getObjectManager().registerParticle(particle);
    world.getAudioEngine().playSoundEffect(TR1SoundEffect::Explosion2, particle.get().get());

    const auto dd = location.position - world.getObjectManager().getLara().m_state.location.position;
    const auto d = util::square(dd.X) + util::square(dd.Y) + util::square(dd.Z);
    if(d < util::square(1024_len))
    {
      world.getObjectManager().getLara().m_state.health
        -= 100_hp * (util::square(1_sectors) - d) / util::square(1_sectors);
      world.getObjectManager().getLara().m_state.is_hit = true;
    }

    return false;
  }
  else if(world.getObjectManager().getLara().isNearInexact(location.position, 200_len))
  {
    world.getObjectManager().getLara().m_state.health -= 100_hp;
    auto particle = gsl::make_shared<ExplosionParticle>(location, world, fall_speed, angle);
    if(!withoutParent())
      setParent(particle, location.room->node);
    world.getObjectManager().registerParticle(particle);
    world.getAudioEngine().playSoundEffect(TR1SoundEffect::Explosion2, particle.get().get());

    if(!world.getObjectManager().getLara().isDead())
    {
      world.getObjectManager().getLara().playSoundEffect(TR1SoundEffect::LaraHurt);
      world.getObjectManager().getLara().forceSourcePosition = &particle->location.position;
      world.getObjectManager().getLara().explosionStumblingDuration = 5_frame;
    }

    world.getObjectManager().getLara().m_state.is_hit = true;
    angle.Y = world.getObjectManager().getLara().m_state.rotation.Y;
    speed = world.getObjectManager().getLara().m_state.speed;
    timePerSpriteFrame = 0;
    negSpriteFrameId = 0;
    return false;
  }

  applyTransform();
  return true;
}

LavaParticle::LavaParticle(const Location& location, world::World& world)
    : Particle{"lava", TR1ItemId::LavaParticles, location, world, render::scene::SpriteMaterialMode::Billboard}
{
  angle.Y = util::rand15(180_deg) * 2;
  speed = util::rand15(32_spd);
  fall_speed = -util::rand15(165_spd);
  negSpriteFrameId = util::rand15(int16_t{-4});
}

bool LavaParticle::update(world::World& world)
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
    world.getObjectManager().getLara().m_state.health -= 10_hp;
    world.getObjectManager().getLara().m_state.is_hit = true;
    return false;
  }

  applyTransform();
  return true;
}

SparkleParticle::SparkleParticle(const Location& location, world::World& world)
    : Particle{"sparkles", TR1ItemId::Sparkles, location, world, render::scene::SpriteMaterialMode::Billboard}
{
}

bool SparkleParticle::update(world::World&)
{
  applyTransform();

  ++timePerSpriteFrame;
  if(timePerSpriteFrame != 1)
    return true;

  --negSpriteFrameId;
  timePerSpriteFrame = 0;
  return gsl::narrow<size_t>(-negSpriteFrameId) < getLength();
}

MuzzleFlashParticle::MuzzleFlashParticle(const Location& location, world::World& world, const core::Angle& yAngle)
    : Particle{"muzzleflash", TR1ItemId::MuzzleFlash, location, world, render::scene::SpriteMaterialMode::YAxisBound}
{
  angle.Y = yAngle;
  timePerSpriteFrame = 3;
  setShade(core::Shade{core::Shade::type{4096}});
}

bool MuzzleFlashParticle::update(world::World&)
{
  --timePerSpriteFrame;
  if(timePerSpriteFrame == 0)
    return false;

  angle.Z = util::rand15s(+180_deg);
  applyTransform();
  return true;
}

ExplosionParticle::ExplosionParticle(const Location& location,
                                     world::World& world,
                                     const core::Speed& fallSpeed,
                                     const core::TRRotation& angle)
    : Particle{"explosion", TR1ItemId::Explosion, location, world, render::scene::SpriteMaterialMode::Billboard}
{
  fall_speed = fallSpeed;
  this->angle = angle;
}

bool ExplosionParticle::update(world::World&)
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

  applyTransform();
  return true;
}

SmokeParticle::SmokeParticle(const Location& location, world::World& world, const core::TRRotation& rotation)
    : Particle{"smoke", TR1ItemId::Smoke, location, world, render::scene::SpriteMaterialMode::YAxisBound}
{
  angle = rotation;
}

bool SmokeParticle::update(world::World&)
{
  applyTransform();

  ++timePerSpriteFrame;
  if(timePerSpriteFrame < 3)
    return true;

  timePerSpriteFrame = 0;
  nextFrame();
  return gsl::narrow<size_t>(-negSpriteFrameId) < getLength();
}

RicochetParticle::RicochetParticle(const Location& location, world::World& world)
    : Particle{"ricochet", TR1ItemId::Ricochet, location, world, render::scene::SpriteMaterialMode::YAxisBound}
{
  timePerSpriteFrame = 4;

  const int n = util::rand15(3);
  for(int i = 0; i < n; ++i)
    nextFrame();
}

bool RicochetParticle::update(world::World&)
{
  applyTransform();

  --timePerSpriteFrame;
  return timePerSpriteFrame != 0;
}

gslu::nn_shared<Particle> createMuzzleFlash(world::World& world,
                                            const Location& location,
                                            const core::Speed& /*speed*/,
                                            const core::Angle& angle)
{
  auto particle = gsl::make_shared<MuzzleFlashParticle>(location, world, angle);
  setParent(particle, location.room->node);
  return particle;
}

gslu::nn_shared<Particle> createMutantBullet(world::World& world,
                                             const Location& location,
                                             const core::Speed& /*speed*/,
                                             const core::Angle& angle)
{
  auto particle = gsl::make_shared<MutantBulletParticle>(location, world, angle);
  setParent(particle, location.room->node);
  return particle;
}

gslu::nn_shared<Particle> createMutantGrenade(world::World& world,
                                              const Location& location,
                                              const core::Speed& /*speed*/,
                                              const core::Angle& angle)
{
  auto particle = gsl::make_shared<MutantGrenadeParticle>(location, world, angle);
  setParent(particle, location.room->node);
  return particle;
}

gslu::nn_shared<Particle>
  createBloodSplat(world::World& world, const Location& location, const core::Speed& speed, const core::Angle& angle)
{
  auto particle = gsl::make_shared<BloodSplatterParticle>(location, speed, angle, world);
  setParent(particle, location.room->node);
  return particle;
}
} // namespace engine
