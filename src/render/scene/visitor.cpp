#include "visitor.h"

#include "node.h"
#include "renderable.h"
#include "rendercontext.h"

#include <algorithm>
#include <gl/debuggroup.h>
#include <glm/geometric.hpp>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace render::scene
{
void Visitor::visit(const Node& node)
{
  if(!node.isVisible())
    return;
  if(const auto& vp = m_context->getViewProjection(); vp.has_value() && node.canBeCulled(*vp))
    return;

  m_context->pushState(node.getRenderState());
  node.accept(*this);
  m_context->popState();
}

void Visitor::add(const gsl::not_null<const Node*>& node)
{
  if(node->getRenderable() != nullptr)
    m_nodes.emplace_back(node, m_context->getCurrentState());
}

void Visitor::render(const std::optional<glm::vec3>& camera) const
{
  if(camera.has_value())
  {
    // logic: first order by render order, then order by distance to camera back-to-front
    std::sort(m_nodes.begin(),
              m_nodes.end(),
              [this, camera](const RenderableInfo& a, const RenderableInfo& b)
              {
                if(auto aOrder = std::get<0>(a)->getRenderOrder(), bOrder = std::get<0>(b)->getRenderOrder();
                   aOrder != bOrder)
                {
                  return aOrder < bOrder;
                }

                auto da = glm::distance(std::get<0>(a)->getTranslationWorld(), *camera);
                auto db = glm::distance(std::get<0>(b)->getTranslationWorld(), *camera);
                return m_backToFront ? db > da : da > db;
              });
  }

  for(const auto& [node, state] : m_nodes)
  {
    SOGLB_DEBUGGROUP(node->getName());
    m_context->pushState(state);
    node->getRenderable()->render(node.get(), *m_context);
    m_context->popState();
  }
}

Visitor::~Visitor() = default;
} // namespace render::scene
