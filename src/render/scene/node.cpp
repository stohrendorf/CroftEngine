#include "node.h"

#include "rendercontext.h"
#include "visitor.h"

#include <gl/renderstate.h>
#include <optional>

namespace render::scene
{
Node::~Node()
{
  if(auto p = m_parent.lock())
  {
    const auto it = std::find_if(p->m_children.begin(),
                                 p->m_children.end(),
                                 [this](const gslu::nn_shared<Node>& node)
                                 {
                                   return node.get().get() == this;
                                 });
    if(it != p->m_children.end())
      p->m_children.erase(it);
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
  const auto [xy, size] = getCombinedScissors();
  state.setScissorRegion(xy, size);
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
} // namespace render::scene
