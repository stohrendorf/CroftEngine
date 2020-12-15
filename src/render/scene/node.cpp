#include "node.h"

#include "camera.h"
#include "scene.h"

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
Scene* Node::getScene() const
{
  if(m_scene != nullptr)
    return m_scene;

  if(const auto p = getParent().lock())
  {
    const auto scene = p->getScene();
    if(scene)
      return scene;
  }
  return nullptr;
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
} // namespace render::scene
