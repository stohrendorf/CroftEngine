#include "particle.h"

#include "engine/objects/laraobject.h"
#include "loader/file/rendermeshdata.h"
#include "render/scene/sprite.h"

#include <utility>

namespace engine
{
void Particle::initRenderables(Engine& engine, const float scale)
{
  if(const auto& modelType = engine.findAnimatedModelForType(object_number))
  {
    for(const auto& bone : modelType->bones)
    {
      loader::file::RenderMeshDataCompositor compositor;
      compositor.append(*bone.mesh);
      m_renderables.emplace_back(compositor.toMesh(*engine.getMaterialManager(), false, {}));
    }
  }
  else if(const auto& spriteSequence = engine.findSpriteSequenceForType(object_number))
  {
    shade = core::Shade{core::Shade::type{4096}};

    for(const loader::file::Sprite& spr : spriteSequence->sprites)
    {
      auto mesh = render::scene::createSpriteMesh(float(spr.x0) * scale,
                                                  float(-spr.y0) * scale,
                                                  float(spr.x1) * scale,
                                                  float(-spr.y1) * scale,
                                                  spr.t0,
                                                  spr.t1,
                                                  engine.getMaterialManager()->getSprite(),
                                                  spr.texture_id.get_as<int32_t>());
      m_renderables.emplace_back(std::move(mesh));
    }

    bindSpritePole(*this, render::scene::SpritePole::Y);
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
  return pos.position.toRenderSystem();
}

Particle::Particle(const std::string& id,
                   const core::TypeId objectNumber,
                   const gsl::not_null<const loader::file::Room*>& room,
                   Engine& engine,
                   const std::shared_ptr<render::scene::Renderable>& renderable,
                   float scale)
    : Node{id}
    , Emitter{&engine.getSoundEngine()}
    , pos{room}
    , object_number{objectNumber}
{
  if(renderable == nullptr)
  {
    initRenderables(engine, scale);
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
                   core::RoomBoundPosition pos,
                   Engine& engine,
                   const std::shared_ptr<render::scene::Renderable>& renderable,
                   float scale)
    : Node{id}
    , Emitter{&engine.getSoundEngine()}
    , pos{std::move(pos)}
    , object_number{objectNumber}
{
  if(renderable == nullptr)
  {
    initRenderables(engine, scale);
  }
  else
  {
    m_renderables.emplace_back(renderable);
    setRenderable(m_renderables.front());
    m_lighting.bind(*this);
  }
}

bool BloodSplatterParticle::update(Engine& engine)
{
  pos.position += util::pitch(speed * 1_frame, angle.Y);
  ++timePerSpriteFrame;
  if(timePerSpriteFrame != 4)
    return true;

  timePerSpriteFrame = 0;
  nextFrame();
  if(negSpriteFrameId <= engine.findSpriteSequenceForType(object_number)->length)
    return false;

  applyTransform();
  return true;
}

bool SplashParticle::update(Engine& engine)
{
  nextFrame();

  if(negSpriteFrameId <= engine.findSpriteSequenceForType(object_number)->length)
  {
    return false;
  }

  pos.position += util::pitch(speed * 1_frame, angle.Y);

  applyTransform();
  return true;
}

bool BubbleParticle::update(Engine& engine)
{
  angle.X += 13_deg;
  angle.Y += 9_deg;
  pos.position += util::pitch(11_len, angle.Y, -speed * 1_frame);
  auto sector = findRealFloorSector(pos.position, &pos.room);
  if(sector == nullptr || !pos.room->isWaterRoom())
  {
    return false;
  }

  if(const auto ceiling = HeightInfo::fromCeiling(sector, pos.position, engine.getObjects()).y;
     ceiling == -core::HeightLimit || pos.position.Y <= ceiling)
  {
    return false;
  }

  applyTransform();
  return true;
}

FlameParticle::FlameParticle(const core::RoomBoundPosition& pos, Engine& engine, bool randomize)
    : Particle{"flame", TR1ItemId::Flame, pos, engine}
{
  timePerSpriteFrame = 0;
  negSpriteFrameId = 0;
  shade = core::Shade{core::Shade::type{4096}};

  if(randomize)
  {
    timePerSpriteFrame = -int(util::rand15(engine.getLara().getSkeleton()->getBoneCount())) - 1;
    for(auto n = util::rand15(getLength()); n != 0; --n)
      nextFrame();
  }
}

bool FlameParticle::update(Engine& engine)
{
  nextFrame();
  if(negSpriteFrameId <= engine.findSpriteSequenceForType(object_number)->length)
    negSpriteFrameId = 0;

  if(timePerSpriteFrame >= 0)
  {
    engine.getAudioEngine().playSound(TR1SoundId::Burning, this);
    if(timePerSpriteFrame != 0)
    {
      --timePerSpriteFrame;
      applyTransform();
      return true;
    }

    if(engine.getLara().isNear(*this, 600_len))
    {
      // it's hot here, isn't it?
      engine.getLara().m_state.health -= 3_hp;
      engine.getLara().m_state.is_hit = true;

      const auto distSq = util::square(engine.getLara().m_state.position.position.X - pos.position.X)
                          + util::square(engine.getLara().m_state.position.position.Z - pos.position.Z);
      if(distSq < util::square(300_len))
      {
        timePerSpriteFrame = 100;

        const auto particle = std::make_shared<FlameParticle>(pos, engine);
        particle->timePerSpriteFrame = -1;
        setParent(particle, pos.room->node);
        engine.getParticles().emplace_back(particle);
      }
    }
  }
  else
  {
    // burn baby burn

    pos.position = {0_len, 0_len, 0_len};
    if(timePerSpriteFrame == -1)
    {
      pos.position.Y = -100_len;
    }
    else
    {
      pos.position.Y = 0_len;
    }

    const auto itemSpheres = engine.getLara().getSkeleton()->getBoneCollisionSpheres(
      engine.getLara().m_state, *engine.getLara().getSkeleton()->getInterpolationInfo().getNearestFrame(), nullptr);

    pos.position
      = core::TRVec{glm::vec3{translate(itemSpheres.at(-timePerSpriteFrame - 1).m, pos.position.toRenderSystem())[3]}};

    if(const auto waterHeight = pos.room->getWaterSurfaceHeight(pos);
       !waterHeight.has_value() || waterHeight.value() >= pos.position.Y)
    {
      engine.getAudioEngine().playSound(TR1SoundId::Burning, this);
      engine.getLara().m_state.health -= 3_hp;
      engine.getLara().m_state.is_hit = true;
    }
    else
    {
      timePerSpriteFrame = 0;
      engine.getAudioEngine().stopSound(TR1SoundId::Burning, this);
      return false;
    }
  }

  applyTransform();
  return true;
}

bool MeshShrapnelParticle::update(Engine& engine)
{
  angle.X += 5_deg;
  angle.Z += 10_deg;
  fall_speed += core::Gravity * 1_frame;

  pos.position += util::pitch(speed * 1_frame, angle.Y, fall_speed * 1_frame);

  const auto sector = findRealFloorSector(pos.position, &pos.room);
  const auto ceiling = HeightInfo::fromCeiling(sector, pos.position, engine.getObjects()).y;
  if(ceiling > pos.position.Y)
  {
    pos.position.Y = ceiling;
    fall_speed = -fall_speed;
  }

  const auto floor = HeightInfo::fromFloor(sector, pos.position, engine.getObjects()).y;

  bool explode = false;

  if(floor <= pos.position.Y)
  {
    if(m_damageRadius <= 0_len)
      return false;

    explode = true;
  }
  else if(engine.getLara().isNear(*this, 2 * m_damageRadius))
  {
    engine.getLara().m_state.is_hit = true;
    if(m_damageRadius <= 0_len)
      return false;

    engine.getLara().m_state.health -= m_damageRadius * 1_hp / 1_len;
    explode = true;

    engine.getLara().forceSourcePosition = &pos.position;
    engine.getLara().explosionStumblingDuration = 5_frame;
  }

  setParent(this, pos.room->node);
  applyTransform();

  if(!explode)
    return true;

  const auto particle = std::make_shared<ExplosionParticle>(pos, engine, fall_speed, angle);
  setParent(particle, pos.room->node);
  engine.getParticles().emplace_back(particle);
  engine.getAudioEngine().playSound(TR1SoundId::Explosion2, particle.get());
  return false;
}

void MutantAmmoParticle::aimLaraChest(Engine& engine)
{
  const auto d = engine.getLara().m_state.position.position - pos.position;
  const auto bbox = engine.getLara().getSkeleton()->getBoundingBox();
  angle.X
    = util::rand15s(256_au)
      - angleFromAtan(bbox.maxY + (bbox.minY - bbox.maxY) * 3 / 4 + d.Y, sqrt(util::square(d.X) + util::square(d.Z)));
  angle.Y = util::rand15s(256_au) + angleFromAtan(d.X, d.Z);
}

bool MutantBulletParticle::update(Engine& engine)
{
  pos.position += util::yawPitch(speed * 1_frame, angle);
  const auto sector = loader::file::findRealFloorSector(pos);
  setParent(this, pos.room->node);
  if(HeightInfo::fromFloor(sector, pos.position, engine.getObjects()).y <= pos.position.Y
     || HeightInfo::fromCeiling(sector, pos.position, engine.getObjects()).y >= pos.position.Y)
  {
    auto particle = std::make_shared<RicochetParticle>(pos, engine);
    particle->timePerSpriteFrame = 6;
    setParent(particle, pos.room->node);
    engine.getParticles().emplace_back(particle);
    engine.getAudioEngine().playSound(TR1SoundId::Ricochet, particle.get());
    return false;
  }
  else if(engine.getLara().isNear(*this, 200_len))
  {
    engine.getLara().m_state.health -= 30_hp;
    auto particle = std::make_shared<BloodSplatterParticle>(pos, speed, angle.Y, engine);
    setParent(particle, pos.room->node);
    engine.getParticles().emplace_back(particle);
    engine.getAudioEngine().playSound(TR1SoundId::BulletHitsLara, particle.get());
    engine.getLara().m_state.is_hit = true;
    angle.Y = engine.getLara().m_state.rotation.Y;
    speed = engine.getLara().m_state.speed;
    timePerSpriteFrame = 0;
    negSpriteFrameId = 0;
    return false;
  }

  applyTransform();
  return true;
}

bool MutantGrenadeParticle::update(Engine& engine)
{
  pos.position += util::yawPitch(speed * 1_frame, angle);
  const auto sector = loader::file::findRealFloorSector(pos);
  setParent(this, pos.room->node);
  if(HeightInfo::fromFloor(sector, pos.position, engine.getObjects()).y <= pos.position.Y
     || HeightInfo::fromCeiling(sector, pos.position, engine.getObjects()).y >= pos.position.Y)
  {
    auto particle = std::make_shared<ExplosionParticle>(pos, engine, fall_speed, angle);
    setParent(particle, pos.room->node);
    engine.getParticles().emplace_back(particle);
    engine.getAudioEngine().playSound(TR1SoundId::Explosion2, particle.get());

    const auto dd = pos.position - engine.getLara().m_state.position.position;
    const auto d = util::square(dd.X) + util::square(dd.Y) + util::square(dd.Z);
    if(d < util::square(1024_len))
    {
      engine.getLara().m_state.health -= 100_hp * (util::square(1024_len) - d) / util::square(1024_len);
      engine.getLara().m_state.is_hit = true;
    }

    return false;
  }
  else if(engine.getLara().isNear(*this, 200_len))
  {
    engine.getLara().m_state.health -= 100_hp;
    auto particle = std::make_shared<ExplosionParticle>(pos, engine, fall_speed, angle);
    setParent(particle, pos.room->node);
    engine.getParticles().emplace_back(particle);
    engine.getAudioEngine().playSound(TR1SoundId::Explosion2, particle.get());

    if(engine.getLara().m_state.health > 0_hp)
    {
      engine.getLara().playSoundEffect(TR1SoundId::LaraHurt);
      engine.getLara().forceSourcePosition = &particle->pos.position;
      engine.getLara().explosionStumblingDuration = 5_frame;
    }

    engine.getLara().m_state.is_hit = true;
    angle.Y = engine.getLara().m_state.rotation.Y;
    speed = engine.getLara().m_state.speed;
    timePerSpriteFrame = 0;
    negSpriteFrameId = 0;
    return false;
  }

  applyTransform();
  return true;
}
bool LavaParticle::update(Engine& engine)
{
  fall_speed += core::Gravity * 1_frame;
  pos.position += util::pitch(speed * 1_frame, angle.Y, fall_speed * 1_frame);

  const auto sector = loader::file::findRealFloorSector(pos);
  setParent(this, pos.room->node);
  if(HeightInfo::fromFloor(sector, pos.position, engine.getObjects()).y <= pos.position.Y
     || HeightInfo::fromCeiling(sector, pos.position, engine.getObjects()).y > pos.position.Y)
  {
    return false;
  }

  if(engine.getLara().isNear(*this, 200_len))
  {
    engine.getLara().m_state.health -= 10_hp;
    engine.getLara().m_state.is_hit = true;
    return false;
  }

  return true;
}
} // namespace engine
