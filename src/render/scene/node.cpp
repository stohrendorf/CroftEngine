#include "node.h"

#include "camera.h"
#include "renderable.h"
#include "rendercontext.h"

#include <gl/debuggroup.h>

namespace render::scene
{
Node::~Node()
{
  if(auto p = m_parent.lock())
  {
    const auto it
      = std::find_if(p->m_children.begin(),
                     p->m_children.end(),
                     [this](const gsl::not_null<std::shared_ptr<Node>>& node) { return node.get().get() == this; });
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

void Node::accept(Visitor& visitor)
{
  SOGLB_DEBUGGROUP(getName());

  const auto renderOnce = [this, &visitor]()
  {
    if(m_renderable != nullptr)
    {
      visitor.getContext().setCurrentNode(this);
      [[maybe_unused]] const bool rendered = m_renderable->render(visitor.getContext());
      if constexpr(Visitor::FlushAfterEachRender)
      {
        if(rendered)
        {
          GL_ASSERT(gl::api::finish());
        }
      }
    }

    for(const auto& node : m_children)
    {
      visitor.visit(*node);
    }
  };

  auto state = visitor.getContext().getCurrentState();
  state.setScissorTest(true);
  const auto [xy, size] = getCombinedScissors();
  state.setScissorRegion(xy, size);
  visitor.getContext().pushState(state);
  renderOnce();
  visitor.getContext().popState();

  visitor.getContext().setCurrentNode(nullptr);
}
} // namespace render::scene
