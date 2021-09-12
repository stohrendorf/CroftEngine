#include "particle.h"

#include "audio/soundengine.h"
#include "audioengine.h"
#include "objects/laraobject.h"
#include "presenter.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "skeletalmodelnode.h"
#include "world/rendermeshdata.h"
#include "world/world.h"

#include <utility>

namespace engine
{
void Particle::initRenderables(world::World& world, bool billboard)
{
  if(const auto& modelType = world.findAnimatedModelForType(object_number))
  {
    for(const auto& bone : modelType->bones)
    {
      world::RenderMeshDataCompositor compositor;
      compositor.append(*bone.mesh);
      m_renderables.emplace_back(compositor.toMesh(*world.getPresenter().getMaterialManager(), false, {}));
    }
  }
  else if(const auto& spriteSequence = world.findSpriteSequenceForType(object_number))
  {
    for(const world::Sprite& spr : spriteSequence->sprites)
    {
      m_renderables.emplace_back(billboard ? spr.billboardMesh : spr.yBoundMesh);
    }
  }
  else
  {
    BOOST_LOG_TRIVIAL(warning) << "Missing sprite/model referenced by particle: "
                               << toString(static_cast<TR1ItemId>(object_number.get()));
    return;
  }

  if(!m_renderables.empty())
  {
    setRenderable(m_renderables.front());
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
                   bool billboard,
                   const std::shared_ptr<render::scene::Renderable>& renderable)
    : Node{id}
    , Emitter{world.getPresenter().getSoundEngine().get()}
    , location{room}
    , object_number{objectNumber}
{
  if(renderable == nullptr)
  {
    initRenderables(world, billboard);
  }
  else
  {
    m_renderables.emplace_back(renderable);
    setRenderable(m_renderables.front());
    m_lighting.bind(*this);
  }
}

Particle::Particle(const std::string& id,
                   const core::TypeId& objectNumber,
                   Location location,
                   world::World& world,
                   bool billboard,
                   const std::shared_ptr<render::scene::Renderable>& renderable)
    : Node{id}
    , Emitter{world.getPresenter().getSoundEngine().get()}
    , location{std::move(location)}
    , object_number{objectNumber}
{
  if(renderable == nullptr)
  {
    initRenderables(world, billboard);
  }
  else
  {
    m_renderables.emplace_back(renderable);
    setRenderable(m_renderables.front());
    m_lighting.bind(*this);
  }
}

void Particle::applyTransform()
{
  location.updateRoom();
  m_lighting.update(m_shade.value_or(core::Shade{core::Shade::type{-1}}), *location.room);
  const glm::vec3 tr = location.position.toRenderSystem() - location.room->position.toRenderSystem();
  setLocalMatrix(translate(glm::mat4{1.0f}, tr) * angle.toMatrix());
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
    : Particle{"splash", TR1ItemId::Splash, location, world, false}
{
  if(!waterfall)
  {
    speed = util::rand15(128_spd);
    angle.Y = core::auToAngle(2 * util::rand15s());
  }
  else
  {
    this->location.position.X += util::rand15s(core::SectorSize);
    this->location.position.Z += util::rand15s(core::SectorSize);
  }
  getRenderState().setScissorTest(false);
}

bool BubbleParticle::update(world::World& world)
{
  angle.X += 13_deg;
  angle.Y += 9_deg;
  location.position += util::pitch(11_len, angle.Y, -speed * 1_frame);
  auto sector = location.updateRoom();
  if(!location.room->isWaterRoom)
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
    : Particle{"flame", TR1ItemId::Flame, location, world, false}
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
    if(lara.isNear(*this, 600_len))
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
                        [](const gsl::not_null<std::shared_ptr<Particle>>& particle)
                        { return particle->object_number == TR1ItemId::Flame && particle->timePerSpriteFrame == -1; });

        if(!alreadyAttachedToLara)
        {
          const auto particle = std::make_shared<FlameParticle>(location, world);
          particle->timePerSpriteFrame = -1;
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
  else if(auto& lara = world.getObjectManager().getLara(); lara.isNear(*this, 2 * m_damageRadius))
  {
    lara.m_state.is_hit = true;
    if(m_damageRadius <= 0_len)
      return false;

    lara.m_state.health -= m_damageRadius * 1_hp / 1_len;
    explode = true;

    lara.forceSourcePosition = &location.position;
    lara.explosionStumblingDuration = 5_frame;
  }

  setParent(this, location.room->node);
  applyTransform();

  if(!explode)
    return true;

  const auto particle = std::make_shared<ExplosionParticle>(location, world, fall_speed, angle);
  setParent(particle, location.room->node);
  world.getObjectManager().registerParticle(particle);
  world.getAudioEngine().playSoundEffect(TR1SoundEffect::Explosion2, particle.get());
  return false;
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
  setParent(this, location.room->node);
  if(HeightInfo::fromFloor(sector, location.position, world.getObjectManager().getObjects()).y <= location.position.Y
     || HeightInfo::fromCeiling(sector, location.position, world.getObjectManager().getObjects()).y
          >= location.position.Y)
  {
    auto particle = std::make_shared<RicochetParticle>(location, world);
    particle->timePerSpriteFrame = 6;
    setParent(particle, location.room->node);
    world.getObjectManager().registerParticle(particle);
    world.getAudioEngine().playSoundEffect(TR1SoundEffect::Ricochet, particle.get());
    return false;
  }
  else if(world.getObjectManager().getLara().isNear(*this, 200_len))
  {
    auto& laraState = world.getObjectManager().getLara().m_state;
    laraState.health -= 30_hp;
    auto particle = std::make_shared<BloodSplatterParticle>(location, speed, angle.Y, world);
    setParent(particle, location.room->node);
    world.getObjectManager().registerParticle(particle);
    world.getAudioEngine().playSoundEffect(TR1SoundEffect::BulletHitsLara, particle.get());
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
  setParent(this, location.room->node);
  if(HeightInfo::fromFloor(sector, location.position, world.getObjectManager().getObjects()).y <= location.position.Y
     || HeightInfo::fromCeiling(sector, location.position, world.getObjectManager().getObjects()).y
          >= location.position.Y)
  {
    auto particle = std::make_shared<ExplosionParticle>(location, world, fall_speed, angle);
    setParent(particle, location.room->node);
    world.getObjectManager().registerParticle(particle);
    world.getAudioEngine().playSoundEffect(TR1SoundEffect::Explosion2, particle.get());

    const auto dd = location.position - world.getObjectManager().getLara().m_state.location.position;
    const auto d = util::square(dd.X) + util::square(dd.Y) + util::square(dd.Z);
    if(d < util::square(1024_len))
    {
      world.getObjectManager().getLara().m_state.health
        -= 100_hp * (util::square(core::SectorSize) - d) / util::square(core::SectorSize);
      world.getObjectManager().getLara().m_state.is_hit = true;
    }

    return false;
  }
  else if(world.getObjectManager().getLara().isNear(*this, 200_len))
  {
    world.getObjectManager().getLara().m_state.health -= 100_hp;
    auto particle = std::make_shared<ExplosionParticle>(location, world, fall_speed, angle);
    setParent(particle, location.room->node);
    world.getObjectManager().registerParticle(particle);
    world.getAudioEngine().playSoundEffect(TR1SoundEffect::Explosion2, particle.get());

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

bool LavaParticle::update(world::World& world)
{
  fall_speed += core::Gravity * 1_frame;
  location.position += util::pitch(speed * 1_frame, angle.Y, fall_speed * 1_frame);

  const auto sector = location.updateRoom();
  setParent(this, location.room->node);
  if(HeightInfo::fromFloor(sector, location.position, world.getObjectManager().getObjects()).y <= location.position.Y
     || HeightInfo::fromCeiling(sector, location.position, world.getObjectManager().getObjects()).y
          > location.position.Y)
  {
    return false;
  }

  if(world.getObjectManager().getLara().isNear(*this, 200_len))
  {
    world.getObjectManager().getLara().m_state.health -= 10_hp;
    world.getObjectManager().getLara().m_state.is_hit = true;
    return false;
  }

  applyTransform();
  return true;
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

bool MuzzleFlashParticle::update(world::World&)
{
  --timePerSpriteFrame;
  if(timePerSpriteFrame == 0)
    return false;

  angle.Z = util::rand15s(+180_deg);
  applyTransform();
  return true;
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

bool RicochetParticle::update(world::World&)
{
  applyTransform();

  --timePerSpriteFrame;
  return timePerSpriteFrame != 0;
}

gsl::not_null<std::shared_ptr<Particle>> createMuzzleFlash(world::World& world,
                                                           const Location& location,
                                                           const core::Speed& /*speed*/,
                                                           const core::Angle& angle)
{
  auto particle = std::make_shared<MuzzleFlashParticle>(location, world, angle);
  setParent(particle, location.room->node);
  return particle;
}

gsl::not_null<std::shared_ptr<Particle>> createMutantBullet(world::World& world,
                                                            const Location& location,
                                                            const core::Speed& /*speed*/,
                                                            const core::Angle& angle)
{
  auto particle = std::make_shared<MutantBulletParticle>(location, world, angle);
  setParent(particle, location.room->node);
  return particle;
}

gsl::not_null<std::shared_ptr<Particle>> createMutantGrenade(world::World& world,
                                                             const Location& location,
                                                             const core::Speed& /*speed*/,
                                                             const core::Angle& angle)
{
  auto particle = std::make_shared<MutantGrenadeParticle>(location, world, angle);
  setParent(particle, location.room->node);
  return particle;
}

gsl::not_null<std::shared_ptr<Particle>>
  createBloodSplat(world::World& world, const Location& location, const core::Speed& speed, const core::Angle& angle)
{
  auto particle = std::make_shared<BloodSplatterParticle>(location, speed, angle, world);
  setParent(particle, location.room->node);
  return particle;
}
} // namespace engine
