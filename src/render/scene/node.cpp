#include "node.h"

#include "rendercontext.h"
#include "visitor.h"

#include <gl/renderstate.h>
#include <gslu.h>
#include <optional>

namespace render::scene
{
Node::~Node()
{
  if(auto p = m_parent.lock())
  {
    auto& children = p->m_children;
    const auto it = std::find_if(children.begin(),
                                 children.end(),
                                 [this](const gslu::nn_shared<Node>& node)
                                 {
                                   return node.get().get() == this;
                                 });
    if(it != children.end())
      children.erase(it);
  }

  m_parent.reset();

  transformChanged();
}

// NOLINTNEXTLINE(misc-no-recursion)
void Node::transformChanged()
{
  m_dirty = true;

  for(const auto& child : m_children)
  {
    child->transformChanged();
  }
}

void Node::accept(Visitor& visitor) const
{
  auto state = visitor.getContext().getCurrentState();
  state.setScissorTest(visitor.withScissors() && m_renderState.getScissorTest().value_or(true));
  const auto [x, y] = getCombinedScissors();
  state.setScissorRegion({x.min, y.min}, {x.size(), y.size()});
  visitor.getContext().pushState(state);

  if(m_renderable != nullptr)
  {
    visitor.add(gsl::not_null{this});
  }

  for(const auto& child : m_children)
  {
    visitor.visit(*child);
  }
  visitor.getContext().popState();
}

std::tuple<core::Interval<float>, core::Interval<float>> Node::getCombinedScissors() const
{
  if(m_scissors.empty())
  {
    if(const auto p = m_parent.lock())
      return p->getCombinedScissors();
    else
      return {{-1, 1}, {-1, 1}};
  }
  core::Interval<float> xInterval{1, -1};
  core::Interval<float> yInterval{1, -1};
  for(const auto& [x, y] : m_scissors)
  {
    xInterval = xInterval.union_(x);
    yInterval = yInterval.union_(y);
  }
  if(const auto p = m_parent.lock())
  {
    const auto [pX, pY] = p->getCombinedScissors();
    xInterval = xInterval.intersect(pX);
    yInterval = yInterval.intersect(pY);
  }
  return {xInterval, yInterval};
}
} // namespace render::scene
