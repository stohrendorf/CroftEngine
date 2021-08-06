#include "particle.h"

#include "audioengine.h"
#include "objects/laraobject.h"
#include "presenter.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "world/rendermeshdata.h"
#include "world/world.h"

#include <utility>

namespace engine
{
void Particle::initRenderables(world::World& world)
{
  setShade(core::Shade{core::Shade::type{4096}});

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
      m_renderables.emplace_back(spr.mesh);
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
                   const core::TypeId objectNumber,
                   const gsl::not_null<const world::Room*>& room,
                   world::World& world,
                   const std::shared_ptr<render::scene::Renderable>& renderable)
    : Node{id}
    , Emitter{world.getPresenter().getSoundEngine().get()}
    , location{room}
    , object_number{objectNumber}
{
  if(renderable == nullptr)
  {
    initRenderables(world);
  }
  else
  {
    m_renderables.emplace_back(renderable);
    setRenderable(m_renderables.front());
    m_lighting.bind(*this);
  }
}

Particle::Particle(const std::string& id,
                   const core::TypeId objectNumber,
                   Location location,
                   world::World& world,
                   const std::shared_ptr<render::scene::Renderable>& renderable)
    : Node{id}
    , Emitter{world.getPresenter().getSoundEngine().get()}
    , location{std::move(location)}
    , object_number{objectNumber}
{
  if(renderable == nullptr)
  {
    initRenderables(world);
  }
  else
  {
    m_renderables.emplace_back(renderable);
    setRenderable(m_renderables.front());
    m_lighting.bind(*this);
  }
}

bool BloodSplatterParticle::update(world::World& world)
{
  location.position += util::pitch(speed * 1_frame, angle.Y);
  ++timePerSpriteFrame;
  if(timePerSpriteFrame != 4)
    return true;

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
     ceiling == -core::HeightLimit || location.position.Y <= ceiling)
  {
    return false;
  }

  applyTransform();
  return true;
}

FlameParticle::FlameParticle(const Location& location, world::World& world, bool randomize)
    : Particle{"flame", TR1ItemId::Flame, location, world}
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

  if(timePerSpriteFrame >= 0)
  {
    world.getAudioEngine().playSoundEffect(TR1SoundEffect::Burning, this);
    if(timePerSpriteFrame != 0)
    {
      --timePerSpriteFrame;
      applyTransform();
      return true;
    }

    if(world.getObjectManager().getLara().isNear(*this, 600_len))
    {
      // it's hot here, isn't it?
      world.getObjectManager().getLara().m_state.health -= 3_hp;
      world.getObjectManager().getLara().m_state.is_hit = true;

      const auto distSq
        = util::square(world.getObjectManager().getLara().m_state.location.position.X - location.position.X)
          + util::square(world.getObjectManager().getLara().m_state.location.position.Z - location.position.Z);
      if(distSq < util::square(300_len))
      {
        timePerSpriteFrame = 100;

        const auto particle = std::make_shared<FlameParticle>(location, world);
        particle->timePerSpriteFrame = -1;
        setParent(particle, location.room->node);
        world.getObjectManager().registerParticle(particle);
      }
    }
  }
  else
  {
    // burn baby burn

    location.position = {0_len, 0_len, 0_len};
    if(timePerSpriteFrame == -1)
    {
      location.position.Y = -100_len;
    }
    else
    {
      location.position.Y = 0_len;
    }

    const auto itemSpheres = world.getObjectManager().getLara().getSkeleton()->getBoneCollisionSpheres(
      world.getObjectManager().getLara().m_state,
      *world.getObjectManager().getLara().getSkeleton()->getInterpolationInfo().getNearestFrame(),
      nullptr);

    location.position = core::TRVec{
      glm::vec3{translate(itemSpheres.at(-timePerSpriteFrame - 1).m, location.position.toRenderSystem())[3]}};

    if(const auto waterHeight = world::getWaterSurfaceHeight(location);
       !waterHeight.has_value() || waterHeight.value() >= location.position.Y)
    {
      world.getAudioEngine().playSoundEffect(TR1SoundEffect::Burning, this);
      world.getObjectManager().getLara().m_state.health -= 3_hp;
      world.getObjectManager().getLara().m_state.is_hit = true;
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
  else if(world.getObjectManager().getLara().isNear(*this, 2 * m_damageRadius))
  {
    world.getObjectManager().getLara().m_state.is_hit = true;
    if(m_damageRadius <= 0_len)
      return false;

    world.getObjectManager().getLara().m_state.health -= m_damageRadius * 1_hp / 1_len;
    explode = true;

    world.getObjectManager().getLara().forceSourcePosition = &location.position;
    world.getObjectManager().getLara().explosionStumblingDuration = 5_frame;
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
  angle.X
    = util::rand15s(256_au)
      - angleFromAtan(bbox.maxY + (bbox.minY - bbox.maxY) * 3 / 4 + d.Y, sqrt(util::square(d.X) + util::square(d.Z)));
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
    world.getObjectManager().getLara().m_state.health -= 30_hp;
    auto particle = std::make_shared<BloodSplatterParticle>(location, speed, angle.Y, world);
    setParent(particle, location.room->node);
    world.getObjectManager().registerParticle(particle);
    world.getAudioEngine().playSoundEffect(TR1SoundEffect::BulletHitsLara, particle.get());
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
  return true;
}

bool ExplosionParticle::update(world::World&)
{
  ++timePerSpriteFrame;
  if(timePerSpriteFrame == 2)
  {
    timePerSpriteFrame = 0;
    --negSpriteFrameId;
    if(negSpriteFrameId <= 0 || static_cast<size_t>(negSpriteFrameId) <= getLength())
    {
      return false;
    }
  }

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
} // namespace engine
