#include "particlecollection.h"

#include "particle.h"
#include "render/scene/mesh.h"
#include "render/scene/names.h"
#include "world/room.h"

#include <boost/log/trivial.hpp>
#include <gl/debuggroup.h>
#include <gl/vertexbuffer.h>

namespace engine
{
void ParticleCollection::eraseParticle(const std::shared_ptr<Particle>& particle)
{
  if(particle == nullptr)
    return;

  const auto it = std::find_if(m_particles.begin(),
                               m_particles.end(),
                               [particle](const auto& p)
                               {
                                 return particle == p.get();
                               });
  if(it != m_particles.end())
    m_particles.erase(it);

  setParent(gsl::not_null{particle}, nullptr);
}

void ParticleCollection::update(world::World& world)
{
  auto currentParticles = std::move(m_particles);
  for(const auto& particle : currentParticles)
  {
    if(particle->update(world))
    {
      if(!particle->withoutParent())
        setParent(particle, particle->location.room->node);
      m_particles.emplace_back(particle);
    }
    else
    {
      setParent(particle, nullptr);
    }
  }
  if(!m_particles.empty())
  {
    BOOST_LOG_TRIVIAL(debug) << "particles: " << m_particles.size();
  }
}

ParticleCollection::~ParticleCollection() = default;

void InstancedParticleCollection::render(render::scene::RenderContext& context) const
{
  SOGLB_DEBUGGROUP("bubble instances");
  std::map<std::shared_ptr<gl::VertexBuffer<glm::mat4>>,
           std::tuple<std::vector<glm::mat4>, std::shared_ptr<render::scene::Mesh>>>
    buffersData;
  for(const auto& particle : *this)
  {
    setParent(particle, nullptr);
    auto [mesh, buffer] = particle->getCurrentMesh();
    if(buffer == nullptr)
      continue;
    particle->applyTransform();
    if(const auto it = buffersData.find(gsl::not_null{buffer}); it != buffersData.end())
    {
      std::get<0>(it->second).emplace_back(particle->getTransform().modelMatrix);
    }
    else
    {
      buffersData.emplace(gsl::not_null{buffer}, std::tuple{std::vector{particle->getTransform().modelMatrix}, mesh});
    }
  }

  for(const auto& [buffer, dataRenderable] : buffersData)
  {
    auto [data, mesh] = dataRenderable;
    if(data.empty())
      continue;

    buffer->setSubData(data, 0);
    mesh->render(nullptr, context, data.size());
  }
}

void InstancedParticleCollection::setAmbient(const world::Room& room)
{
  m_lighting.ambient = toBrightness(room.ambientShade);
  m_lighting.update(room.ambientShade, room);
}

void InstancedParticleCollection::registerParticle(const gslu::nn_shared<Particle>& particle)
{
  ParticleCollection::registerParticle(particle);
  m_lighting.bind(*particle);
}

void InstancedParticleCollection::registerParticle(gslu::nn_shared<Particle>&& particle)
{
  ParticleCollection::registerParticle(particle);
  m_lighting.bind(*particle);
}
} // namespace engine
