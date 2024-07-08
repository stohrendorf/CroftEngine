#include "particlecollection.h"

#include "core/units.h"
#include "lighting.h"
#include "location.h"
#include "particle.h"
#include "render/scene/mesh.h"
#include "render/scene/node.h"
#include "world/room.h"

#include <algorithm>
#include <gl/debuggroup.h>
#include <gl/vertexbuffer.h>
#include <glm/mat4x4.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

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
}

ParticleCollection::~ParticleCollection() = default;

void InstancedParticleCollection::render(const std::string& roomName,
                                         render::scene::RenderContext& context,
                                         const world::World& world) const
{
  if(empty())
    return;

  SOGLB_DEBUGGROUP(roomName + ":bubble-instances");
  std::map<std::shared_ptr<gl::VertexBuffer<glm::mat4>>, std::tuple<std::vector<glm::mat4>, gslu::nn_shared<Particle>>>
    buffersData;
  for(const auto& particle : *this)
  {
    setParent(particle, nullptr);
    auto buffer = std::get<1>(particle->getCurrentMesh());
    if(buffer == nullptr)
      continue;
    particle->applyTransform();
    if(const auto it = buffersData.find(gsl::not_null{buffer}); it != buffersData.end())
    {
      std::get<0>(it->second).emplace_back(particle->getTransform().modelMatrix);
    }
    else
    {
      buffersData.emplace(gsl::not_null{buffer},
                          std::tuple{std::vector{particle->getTransform().modelMatrix}, particle});
    }
  }

  for(const auto& [buffer, dataRenderable] : buffersData)
  {
    auto [data, particle] = dataRenderable;
    if(data.empty())
      continue;

    m_lighting.bind(*particle, world);
    buffer->setSubData(data, 0);
    auto mesh = std::get<0>(particle->getCurrentMesh());
    mesh->render(particle.get().get(), context, gsl::narrow<gl::api::core::SizeType>(data.size()));
  }
}

void InstancedParticleCollection::setAmbient(const world::Room& room)
{
  m_lighting.ambient = toBrightness(room.ambientShade);
  m_lighting.update(core::Shade{core::Shade::type(-1)}, room);
}
} // namespace engine
