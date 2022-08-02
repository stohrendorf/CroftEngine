#pragma once

#include "lighting.h"

#include <gl/soglb_fwd.h>
#include <gslu.h>
#include <map>
#include <memory>
#include <vector>

namespace engine::world
{
class World;
}

namespace render::scene
{
class Renderable;
class RenderContext;
} // namespace render::scene

namespace engine
{
class Particle;

class ParticleCollection
{
public:
  virtual ~ParticleCollection();

  virtual void registerParticle(const gslu::nn_shared<Particle>& particle)
  {
    m_particles.emplace_back(particle);
  }

  virtual void registerParticle(gslu::nn_shared<Particle>&& particle)
  {
    m_particles.emplace_back(std::move(particle));
  }

  void eraseParticle(const std::shared_ptr<Particle>& particle);

  void update(world::World& world);

  [[nodiscard]] auto begin() const
  {
    return m_particles.begin();
  }

  [[nodiscard]] auto end() const
  {
    return m_particles.end();
  }

private:
  std::vector<gslu::nn_shared<Particle>> m_particles;
};

class InstancedParticleCollection : public ParticleCollection
{
public:
  void render(render::scene::RenderContext& context) const;
  void setAmbient(const world::Room& room);
  void registerParticle(const gslu::nn_shared<engine::Particle>& particle) override;
  void registerParticle(gslu::nn_shared<engine::Particle>&& particle) override;

private:
  Lighting m_lighting;
};
} // namespace engine
