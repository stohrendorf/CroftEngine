#pragma once

#include "audio/emitter.h"
#include "core/angle.h"
#include "core/id.h"
#include "core/units.h"
#include "engine/lighting.h"
#include "items_tr1.h"
#include "location.h"
#include "render/scene/node.h"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <gl/soglb_fwd.h>
#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <optional>
#include <string>
#include <tuple>

namespace render::scene
{
class Mesh;
}

namespace render::material
{
enum class SpriteMaterialMode : uint8_t;
}

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
  core::TRRotation angle;
  core::TypeId object_number;
  core::Speed speed = 0_spd;
  core::Speed fall_speed = 0_spd;
  int16_t negSpriteFrameId = 0;
  int16_t timePerSpriteFrame = 0;
  float scale = 1.0f;

  std::tuple<std::shared_ptr<render::scene::Mesh>, std::shared_ptr<gl::VertexBuffer<glm::mat4>>> getCurrentMesh() const;

private:
  std::deque<std::tuple<gslu::nn_shared<render::scene::Mesh>, std::shared_ptr<gl::VertexBuffer<glm::mat4>>>> m_meshes;
  Lighting m_lighting;
  std::optional<core::Shade> m_shade{std::nullopt};
  bool m_withoutParent;

  void initRenderables(world::World& world, render::material::SpriteMaterialMode mode);

protected:
  void nextFrame();

  size_t getLength() const noexcept
  {
    return m_meshes.size();
  }

  void clearMeshes() noexcept
  {
    m_meshes.clear();
  }

public:
  explicit Particle(const std::string& id,
                    const core::TypeId& objectNumber,
                    const gsl::not_null<const world::Room*>& room,
                    world::World& world,
                    render::material::SpriteMaterialMode mode,
                    bool withoutParent = false,
                    const std::shared_ptr<render::scene::Mesh>& renderable = nullptr);

  explicit Particle(const std::string& id,
                    const core::TypeId& objectNumber,
                    Location location,
                    world::World& world,
                    render::material::SpriteMaterialMode mode,
                    bool withoutParent = false,
                    const std::shared_ptr<render::scene::Mesh>& renderable = nullptr);

  void setShade(const core::Shade& shade) noexcept
  {
    m_shade = shade;
  }

  virtual bool update(world::World& world) = 0;

  glm::vec3 getPosition() const final;

  void applyTransform();

  [[nodiscard]] bool withoutParent() const noexcept
  {
    return m_withoutParent;
  }
};

class BloodSplatterParticle final : public Particle
{
public:
  explicit BloodSplatterParticle(const Location& location,
                                 const core::Speed& speed_,
                                 const core::Angle& angle_,
                                 world::World& world);

  bool update(world::World& world) override;
};

class SplashParticle final : public Particle
{
public:
  explicit SplashParticle(const Location& location, world::World& world, bool waterfall);

  bool update(world::World& world) override;
};

class RicochetParticle final : public Particle
{
public:
  explicit RicochetParticle(const Location& location, world::World& world);

  bool update(world::World& /*world*/) override;
};

class BubbleParticle final : public Particle
{
public:
  explicit BubbleParticle(const Location& location,
                          world::World& world,
                          bool onlyInWater = true,
                          bool instanced = false);

  bool update(world::World& world) override;

  core::Length circleRadius = 11_len;

private:
  bool m_onlyInWater;
};

class SparkleParticle final : public Particle
{
public:
  explicit SparkleParticle(const Location& location, world::World& world);

  bool update(world::World& /*world*/) override;
};

class MuzzleFlashParticle final : public Particle
{
public:
  explicit MuzzleFlashParticle(const Location& location, world::World& world, const core::Angle& yAngle);

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
                             const core::TRRotation& angle);

  bool update(world::World& /*world*/) override;
};

class MeshShrapnelParticle final : public Particle
{
public:
  explicit MeshShrapnelParticle(const Location& location,
                                world::World& world,
                                const gslu::nn_shared<render::scene::Mesh>& renderable,
                                bool torsoBoss,
                                const core::Length& damageRadius);

  bool update(world::World& world) override;

private:
  core::Length m_damageRadius;
};

class MutantAmmoParticle : public Particle
{
protected:
  explicit MutantAmmoParticle(const Location& location, world::World& world, TR1ItemId itemType);

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
  explicit LavaParticle(const Location& location, world::World& world);

  bool update(world::World& world) override;
};

class SmokeParticle final : public Particle
{
public:
  explicit SmokeParticle(const Location& location, world::World& world, const core::TRRotation& rotation);

  bool update(world::World& /*world*/) override;
};

extern gslu::nn_shared<Particle>
  createBloodSplat(world::World& world, const Location& location, const core::Speed& speed, const core::Angle& angle);

extern gslu::nn_shared<Particle> createMuzzleFlash(world::World& world,
                                                   const Location& location,
                                                   const core::Speed& /*speed*/,
                                                   const core::Angle& angle);

extern gslu::nn_shared<Particle> createMutantBullet(world::World& world,
                                                    const Location& location,
                                                    const core::Speed& /*speed*/,
                                                    const core::Angle& angle);

extern gslu::nn_shared<Particle> createMutantGrenade(world::World& world,
                                                     const Location& location,
                                                     const core::Speed& /*speed*/,
                                                     const core::Angle& angle);
} // namespace engine
