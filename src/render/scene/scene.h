#pragma once

#include "node.h"

namespace render::scene
{
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
    m_nodes.emplace_back(node);
  }

  void accept(Visitor& visitor)
  {
    auto ordered = m_nodes;
    std::sort(ordered.begin(),
              ordered.end(),
              [](const gsl::not_null<std::shared_ptr<Node>>& l, const gsl::not_null<std::shared_ptr<Node>>& r) {
                return l->getRenderOrder() < r->getRenderOrder();
              });

    for(auto& node : ordered)
      visitor.visit(*node);
  }

  void clear()
  {
    auto tmp = m_nodes;
    for(const auto& node : tmp)
      setParent(node, nullptr);
  }

private:
  Node::List m_nodes;
};
} // namespace render::scene
