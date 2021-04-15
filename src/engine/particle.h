#pragma once

#include "audio/soundengine.h"
#include "core/angle.h"
#include "core/id.h"
#include "core/vec.h"
#include "engine/lighting.h"
#include "engine/world/room.h"
#include "items_tr1.h"
#include "render/scene/node.h"
#include "util/helpers.h"

#include <deque>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace engine::world
{
class World;
}

namespace engine
{
class Particle
    : public render::scene::Node
    , public audio::Emitter
{
public:
  core::RoomBoundPosition pos;
  core::TRRotation angle{0_deg, 0_deg, 0_deg};
  const core::TypeId object_number;
  core::Speed speed = 0_spd;
  core::Speed fall_speed = 0_spd;
  int16_t negSpriteFrameId = 0;
  int16_t timePerSpriteFrame = 0;

private:
  std::deque<gsl::not_null<std::shared_ptr<render::scene::Renderable>>> m_renderables{};
  Lighting m_lighting;

  void initRenderables(world::World& world);

protected:
  void nextFrame()
  {
    --negSpriteFrameId;

    if(m_renderables.empty())
      return;

    m_renderables.emplace_back(m_renderables.front());
    m_renderables.pop_front();
    setRenderable(m_renderables.front());
  }

  void applyTransform()
  {
    const glm::vec3 tr = pos.position.toRenderSystem() - pos.room->position.toRenderSystem();
    setLocalMatrix(translate(glm::mat4{1.0f}, tr) * angle.toMatrix());
  }

  size_t getLength() const
  {
    return m_renderables.size();
  }

  void clearRenderables()
  {
    m_renderables.clear();
  }

public:
  explicit Particle(const std::string& id,
                    core::TypeId objectNumber,
                    const gsl::not_null<const world::Room*>& room,
                    world::World& world,
                    const std::shared_ptr<render::scene::Renderable>& renderable = nullptr);

  explicit Particle(const std::string& id,
                    core::TypeId objectNumber,
                    core::RoomBoundPosition pos,
                    world::World& world,
                    const std::shared_ptr<render::scene::Renderable>& renderable = nullptr);

  void setShade(const core::Shade& shade)
  {
    m_lighting.update(shade, *pos.room);
  }

  virtual bool update(world::World& world) = 0;

  glm::vec3 getPosition() const final;
};

class BloodSplatterParticle final : public Particle
{
public:
  explicit BloodSplatterParticle(const core::RoomBoundPosition& pos,
                                 const core::Speed& speed_,
                                 const core::Angle& angle_,
                                 world::World& world)
      : Particle{"bloodsplat", TR1ItemId::Blood, pos, world}
  {
    speed = speed_;
    angle.Y = angle_;
  }

  bool update(world::World& world) override;
};

class SplashParticle final : public Particle
{
public:
  explicit SplashParticle(const core::RoomBoundPosition& pos, world::World& world, const bool waterfall)
      : Particle{"splash", TR1ItemId::Splash, pos, world}
  {
    if(!waterfall)
    {
      speed = util::rand15(128_spd);
      angle.Y = core::auToAngle(2 * util::rand15s());
    }
    else
    {
      this->pos.position.X += util::rand15s(core::SectorSize);
      this->pos.position.Z += util::rand15s(core::SectorSize);
    }
  }

  bool update(world::World& world) override;
};

class RicochetParticle final : public Particle
{
public:
  explicit RicochetParticle(const core::RoomBoundPosition& pos, world::World& world)
      : Particle{"ricochet", TR1ItemId::Ricochet, pos, world}
  {
    timePerSpriteFrame = 4;

    const int n = util::rand15(3);
    for(int i = 0; i < n; ++i)
      nextFrame();
  }

  bool update(world::World& /*world*/) override;
};

class BubbleParticle final : public Particle
{
public:
  explicit BubbleParticle(const core::RoomBoundPosition& pos, world::World& world)
      : Particle{"bubble", TR1ItemId::Bubbles, pos, world, nullptr}
  {
    speed = 10_spd + util::rand15(6_spd);

    const int n = util::rand15(3);
    for(int i = 0; i < n; ++i)
      nextFrame();
  }

  bool update(world::World& world) override;
};

class SparkleParticle final : public Particle
{
public:
  explicit SparkleParticle(const core::RoomBoundPosition& pos, world::World& world)
      : Particle{"sparkles", TR1ItemId::Sparkles, pos, world}
  {
  }

  bool update(world::World& /*world*/) override;
};

class MuzzleFlashParticle final : public Particle
{
public:
  explicit MuzzleFlashParticle(const core::RoomBoundPosition& pos, world::World& world, const core::Angle& yAngle)
      : Particle{"muzzleflash", TR1ItemId::MuzzleFlash, pos, world}
  {
    angle.Y = yAngle;
    timePerSpriteFrame = 3;
    setShade(core::Shade{core::Shade::type{4096}});
  }

  bool update(world::World& /*world*/) override;
};

class FlameParticle final : public Particle
{
public:
  explicit FlameParticle(const core::RoomBoundPosition& pos, world::World& world, bool randomize = false);

  bool update(world::World& world) override;
};

class ExplosionParticle final : public Particle
{
public:
  explicit ExplosionParticle(const core::RoomBoundPosition& pos,
                             world::World& world,
                             const core::Speed& fallSpeed,
                             const core::TRRotation& angle)
      : Particle{"explosion", TR1ItemId::Explosion, pos, world}
  {
    fall_speed = fallSpeed;
    this->angle = angle;
  }

  bool update(world::World& /*world*/) override;
};

class MeshShrapnelParticle final : public Particle
{
public:
  explicit MeshShrapnelParticle(const core::RoomBoundPosition& pos,
                                world::World& world,
                                const gsl::not_null<std::shared_ptr<render::scene::Renderable>>& renderable,
                                const bool torsoBoss,
                                const core::Length& damageRadius)
      : Particle{"meshShrapnel", TR1ItemId::MeshShrapnel, pos, world, renderable}
      , m_damageRadius{damageRadius}
  {
    clearRenderables();

    angle.Y = core::Angle{util::rand15s() * 2};
    speed = util::rand15(256_spd);
    fall_speed = util::rand15(256_spd);
    if(!torsoBoss)
    {
      speed /= 2;
      fall_speed /= 2;
    }
  }

  bool update(world::World& world) override;

private:
  const core::Length m_damageRadius;
};

class MutantAmmoParticle : public Particle
{
protected:
  explicit MutantAmmoParticle(const core::RoomBoundPosition& pos, world::World& world, const TR1ItemId itemType)
      : Particle{"mutantAmmo", itemType, pos, world}
  {
  }

  void aimLaraChest(world::World& world);
};

class MutantBulletParticle final : public MutantAmmoParticle
{
public:
  explicit MutantBulletParticle(const core::RoomBoundPosition& pos, world::World& world, const core::Angle& yAngle)
      : MutantAmmoParticle{pos, world, TR1ItemId::MutantBullet}
  {
    speed = 250_spd;
    setShade(core::Shade{core::Shade::type{3584}});
    angle.Y = yAngle;
    aimLaraChest(world);
  }

  bool update(world::World& world) override;
};

class MutantGrenadeParticle final : public MutantAmmoParticle
{
public:
  explicit MutantGrenadeParticle(const core::RoomBoundPosition& pos, world::World& world, const core::Angle& yAngle)
      : MutantAmmoParticle{pos, world, TR1ItemId::MutantGrenade}
  {
    speed = 220_spd;
    angle.Y = yAngle;
    aimLaraChest(world);
  }

  bool update(world::World& world) override;
};

class LavaParticle final : public Particle
{
public:
  explicit LavaParticle(const core::RoomBoundPosition& pos, world::World& world)
      : Particle{"lava", TR1ItemId::LavaParticles, pos, world}
  {
    angle.Y = util::rand15(180_deg) * 2;
    speed = util::rand15(512_spd);
    fall_speed = -util::rand15(165_spd);
    negSpriteFrameId = util::rand15(int16_t{-4});
  }

  bool update(world::World& world) override;
};

class SmokeParticle final : public Particle
{
public:
  explicit SmokeParticle(const core::RoomBoundPosition& pos, world::World& world, const core::TRRotation& rotation)
      : Particle{"smoke", TR1ItemId::Smoke, pos, world}
  {
    angle = rotation;
  }

  bool update(world::World& /*world*/) override;
};

inline gsl::not_null<std::shared_ptr<Particle>> createBloodSplat(world::World& world,
                                                                 const core::RoomBoundPosition& pos,
                                                                 const core::Speed& speed,
                                                                 const core::Angle& angle)
{
  auto particle = std::make_shared<BloodSplatterParticle>(pos, speed, angle, world);
  setParent(particle, pos.room->node);
  return particle;
}
} // namespace engine