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

void Node::updateMatrices() const
{
  if(!m_dirty)
    return;

  m_dirty = false;

  if(const auto p = getParent().lock())
  {
    m_transform.modelMatrix = p->getModelMatrix() * m_localMatrix;
  }
  else
  {
    m_transform.modelMatrix = m_localMatrix;
  }
}

const glm::mat4& Node::getViewMatrix() const
{
  const auto scene = getScene();
  if(const auto camera = scene != nullptr ? scene->getActiveCamera() : nullptr)
  {
    return camera->getViewMatrix();
  }
  else
  {
    static const glm::mat4 identity{1.0f};
    return identity;
  }
}

const glm::mat4& Node::getProjectionMatrix() const
{
  const auto scene = getScene();
  const auto camera = scene != nullptr ? scene->getActiveCamera() : nullptr;
  if(camera != nullptr)
  {
    return camera->getProjectionMatrix();
  }
  else
  {
    static const glm::mat4 identity{1.0f};
    return identity;
  }
}

const glm::mat4& Node::getViewProjectionMatrix() const
{
  const auto scene = getScene();
  const auto camera = scene != nullptr ? scene->getActiveCamera() : nullptr;
  if(camera != nullptr)
  {
    return camera->getViewProjectionMatrix();
  }
  else
  {
    static const glm::mat4 identity{1.0f};
    return identity;
  }
}

void Node::transformChanged()
{
  m_dirty = true;

  for(const auto& child : m_children)
  {
    child->transformChanged();
  }
}
} // namespace render::scene
