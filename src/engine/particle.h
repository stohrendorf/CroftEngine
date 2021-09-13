#pragma once

#include "audio/emitter.h"
#include "core/angle.h"
#include "core/id.h"
#include "engine/lighting.h"
#include "items_tr1.h"
#include "location.h"
#include "render/scene/node.h"
#include "util/helpers.h"

#include <deque>
#include <memory>

namespace engine::world
{
class World;
struct Room;
} // namespace engine::world

namespace engine
{
class Particle
    : public render::scene::Node
    , public audio::Emitter
{
public:
  Location location;
  core::TRRotation angle{};
  const core::TypeId object_number;
  core::Speed speed = 0_spd;
  core::Speed fall_speed = 0_spd;
  int16_t negSpriteFrameId = 0;
  int16_t timePerSpriteFrame = 0;

private:
  std::deque<gsl::not_null<std::shared_ptr<render::scene::Renderable>>> m_renderables{};
  Lighting m_lighting;
  std::optional<core::Shade> m_shade{std::nullopt};

  void initRenderables(world::World& world, bool billboard);

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

  void applyTransform();

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
                    const core::TypeId& objectNumber,
                    const gsl::not_null<const world::Room*>& room,
                    world::World& world,
                    bool billboard,
                    const std::shared_ptr<render::scene::Renderable>& renderable = nullptr);

  explicit Particle(const std::string& id,
                    const core::TypeId& objectNumber,
                    Location location,
                    world::World& world,
                    bool billboard,
                    const std::shared_ptr<render::scene::Renderable>& renderable = nullptr);

  void setShade(const core::Shade& shade)
  {
    m_shade = shade;
  }

  virtual bool update(world::World& world) = 0;

  glm::vec3 getPosition() const final;
};

class BloodSplatterParticle final : public Particle
{
public:
  explicit BloodSplatterParticle(const Location& location,
                                 const core::Speed& speed_,
                                 const core::Angle& angle_,
                                 world::World& world)
      : Particle{"bloodsplat", TR1ItemId::Blood, location, world, true}
  {
    speed = speed_;
    angle.Y = angle_;
  }

  bool update(world::World& world) override;
};

class SplashParticle final : public Particle
{
public:
  explicit SplashParticle(const Location& location, world::World& world, const bool waterfall);

  bool update(world::World& world) override;
};

class RicochetParticle final : public Particle
{
public:
  explicit RicochetParticle(const Location& location, world::World& world)
      : Particle{"ricochet", TR1ItemId::Ricochet, location, world, false}
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
  explicit BubbleParticle(const Location& location, world::World& world)
      : Particle{"bubble", TR1ItemId::Bubbles, location, world, true, nullptr}
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
  explicit SparkleParticle(const Location& location, world::World& world)
      : Particle{"sparkles", TR1ItemId::Sparkles, location, world, true}
  {
  }

  bool update(world::World& /*world*/) override;
};

class MuzzleFlashParticle final : public Particle
{
public:
  explicit MuzzleFlashParticle(const Location& location, world::World& world, const core::Angle& yAngle)
      : Particle{"muzzleflash", TR1ItemId::MuzzleFlash, location, world, false}
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
  explicit FlameParticle(const Location& location, world::World& world, bool randomize = false);

  bool update(world::World& world) override;
};

class ExplosionParticle final : public Particle
{
public:
  explicit ExplosionParticle(const Location& location,
                             world::World& world,
                             const core::Speed& fallSpeed,
                             const core::TRRotation& angle)
      : Particle{"explosion", TR1ItemId::Explosion, location, world, true}
  {
    fall_speed = fallSpeed;
    this->angle = angle;
  }

  bool update(world::World& /*world*/) override;
};

class MeshShrapnelParticle final : public Particle
{
public:
  explicit MeshShrapnelParticle(const Location& location,
                                world::World& world,
                                const gsl::not_null<std::shared_ptr<render::scene::Renderable>>& renderable,
                                const bool torsoBoss,
                                const core::Length& damageRadius)
      : Particle{"meshShrapnel", TR1ItemId::MeshShrapnel, location, world, false, renderable}
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
  explicit MutantAmmoParticle(const Location& location, world::World& world, const TR1ItemId itemType)
      : Particle{"mutantAmmo", itemType, location, world, false}
  {
  }

  void aimLaraChest(world::World& world);
};

class MutantBulletParticle final : public MutantAmmoParticle
{
public:
  explicit MutantBulletParticle(const Location& location, world::World& world, const core::Angle& yAngle)
      : MutantAmmoParticle{location, world, TR1ItemId::MutantBullet}
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
  explicit MutantGrenadeParticle(const Location& location, world::World& world, const core::Angle& yAngle)
      : MutantAmmoParticle{location, world, TR1ItemId::MutantGrenade}
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
  explicit LavaParticle(const Location& location, world::World& world)
      : Particle{"lava", TR1ItemId::LavaParticles, location, world, true}
  {
    angle.Y = util::rand15(180_deg) * 2;
    speed = util::rand15(32_spd);
    fall_speed = -util::rand15(165_spd);
    negSpriteFrameId = util::rand15(int16_t{-4});
  }

  bool update(world::World& world) override;
};

class SmokeParticle final : public Particle
{
public:
  explicit SmokeParticle(const Location& location, world::World& world, const core::TRRotation& rotation)
      : Particle{"smoke", TR1ItemId::Smoke, location, world, false}
  {
    angle = rotation;
  }

  bool update(world::World& /*world*/) override;
};

extern gsl::not_null<std::shared_ptr<Particle>>
  createBloodSplat(world::World& world, const Location& location, const core::Speed& speed, const core::Angle& angle);

extern gsl::not_null<std::shared_ptr<Particle>> createMuzzleFlash(world::World& world,
                                                                  const Location& location,
                                                                  const core::Speed& /*speed*/,
                                                                  const core::Angle& angle);

extern gsl::not_null<std::shared_ptr<Particle>> createMutantBullet(world::World& world,
                                                                   const Location& location,
                                                                   const core::Speed& /*speed*/,
                                                                   const core::Angle& angle);

extern gsl::not_null<std::shared_ptr<Particle>> createMutantGrenade(world::World& world,
                                                                    const Location& location,
                                                                    const core::Speed& /*speed*/,
                                                                    const core::Angle& angle);
} // namespace engine
