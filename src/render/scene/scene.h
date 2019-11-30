#pragma once

#include "node.h"

namespace render::scene
{
class Camera;

class Scene final
{
public:
  explicit Scene() = default;

  Scene(const Scene&) = delete;

  Scene(Scene&&) = delete;

  Scene& operator=(const Scene&) = delete;

  Scene& operator=(Scene&&) = delete;

  ~Scene() = default;

  void addNode(const gsl::not_null<std::shared_ptr<Node>>& node)
  {
    if(node->m_scene == this)
    {
      // The node is already a member of this scene.
      return;
    }

    m_nodes.emplace_back(node);
    node->m_scene = this;
  }

  [[nodiscard]] const std::shared_ptr<Camera>& getActiveCamera() const
  {
    return m_activeCamera;
  }

  void setActiveCamera(const std::shared_ptr<Camera>& camera)
  {
    m_activeCamera = camera;
  }

  void accept(Visitor& visitor)
  {
    for(auto& node : m_nodes)
      visitor.visit(*node);
  }

private:
  std::shared_ptr<Camera> m_activeCamera = nullptr;
  Node::List m_nodes;
};
} // namespace render
