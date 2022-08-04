#pragma once

#include "materialparameteroverrider.h"

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/throw_exception.hpp>
#include <gl/buffer.h>
#include <gl/renderstate.h>
#include <glm/common.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace render::scene
{
class Renderable;
class Visitor;

struct Transform
{
  glm::mat4 modelMatrix{1.0f};
};

class Node : public MaterialParameterOverrider
{
public:
  Node(const Node&) = delete;
  Node(Node&&) = delete;
  Node& operator=(Node&&) = delete;
  Node& operator=(const Node&) = delete;

  using List = std::vector<gslu::nn_shared<Node>>;

  explicit Node(std::string name)
      : m_name{std::move(name)}
      , m_transformBuffer{m_name + "-transform-ubo", gl::api::BufferUsage::StreamDraw, m_transform}
  {
  }

  ~Node() override;

  [[nodiscard]] const std::string& getName() const
  {
    return m_name;
  }

  [[nodiscard]] const std::weak_ptr<Node>& getParent() const
  {
    return m_parent;
  }

  void setVisible(bool visible)
  {
    m_visible = visible;
  }

  [[nodiscard]] bool isVisible() const
  {
    return m_visible;
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  [[nodiscard]] const glm::mat4& getModelMatrix() const
  {
    if(!m_dirty)
      return m_transform.modelMatrix;

    m_dirty = false;

    const auto old = m_transform.modelMatrix;
    if(const auto p = getParent().lock())
    {
      m_transform.modelMatrix = p->getModelMatrix() * m_localMatrix;
    }
    else
    {
      m_transform.modelMatrix = m_localMatrix;
    }
    m_bufferDirty |= m_transform.modelMatrix != old;
    return m_transform.modelMatrix;
  }

  [[nodiscard]] glm::vec3 getTranslationWorld() const
  {
    return {getModelMatrix()[3]};
  }

  [[nodiscard]] const std::shared_ptr<Renderable>& getRenderable() const
  {
    return m_renderable;
  }

  void setRenderable(const std::shared_ptr<Renderable>& renderable)
  {
    m_renderable = renderable;
  }

  [[nodiscard]] const List& getChildren() const
  {
    return m_children;
  }

  [[nodiscard]] List& getChildren()
  {
    return m_children;
  }

  void removeAllChildren()
  {
    for(auto& child : m_children)
      child->m_parent.reset();
    m_children.clear();
  }

  [[nodiscard]] const glm::mat4& getLocalMatrix() const
  {
    return m_localMatrix;
  }

  void setLocalMatrix(const glm::mat4& m)
  {
    m_localMatrix = m;
    transformChanged();
  }

  void accept(Visitor& visitor) const;

  [[nodiscard]] std::shared_ptr<Node> findChild(const Node* node) const
  {
    const auto it = std::find_if(m_children.begin(),
                                 m_children.end(),
                                 [node](const gslu::nn_shared<Node>& ptr)
                                 {
                                   return ptr.get().get() == node;
                                 });

    if(it == m_children.end())
      return nullptr;

    return *it;
  }

  [[nodiscard]] const auto& getTransformBuffer() const
  {
    (void)getModelMatrix(); // update data if dirty
    if(!m_bufferDirty)
      return m_transformBuffer;

    m_bufferDirty = false;
    m_transformBuffer.setSubData(m_transform, 0);
    return m_transformBuffer;
  }

  [[nodiscard]] virtual bool canBeCulled(const glm::mat4& /*viewProjection*/) const
  {
    return false;
  }

  void clear()
  {
    auto tmp = m_children;
    for(const auto& node : tmp)
      setParent(node, nullptr);
  }

  void clearScissors()
  {
    m_scissors.clear();
  }

  void addScissor(const glm::vec2& xy, const glm::vec2& size)
  {
    m_scissors.emplace_back(xy, size);
  }

  std::tuple<glm::vec2, glm::vec2> getCombinedScissors() const
  {
    if(m_scissors.empty())
    {
      if(const auto p = m_parent.lock())
        return p->getCombinedScissors();
      else
        return {{-1, -1}, {2, 2}};
    }
    glm::vec2 min{1, 1};
    glm::vec2 max{-1, -1};
    for(const auto& [xy, size] : m_scissors)
    {
      min = glm::min(min, xy);
      max = glm::max(max, xy + size);
    }
    if(const auto p = m_parent.lock())
    {
      const auto [pXy, pSize] = p->getCombinedScissors();
      min = glm::max(min, pXy);
      max = glm::min(max, pXy + pSize);
    }
    return {min, max - min};
  }

  gl::RenderState& getRenderState()
  {
    return m_renderState;
  }

  const gl::RenderState& getRenderState() const
  {
    return m_renderState;
  }

  [[nodiscard]] auto getRenderOrder() const
  {
    return m_renderOrder;
  }

  void setRenderOrder(int order)
  {
    m_renderOrder = order;
  }

  const auto& getTransform() const
  {
    (void)getModelMatrix(); // update data if dirty
    return m_transform;
  }

private:
  void transformChanged();

  std::string m_name;
  List m_children;
  std::weak_ptr<Node> m_parent{};
  bool m_visible = true;
  std::shared_ptr<Renderable> m_renderable = nullptr;
  glm::mat4 m_localMatrix{1.0f};
  gl::RenderState m_renderState;

  mutable bool m_dirty = false;
  mutable bool m_bufferDirty = true;
  mutable Transform m_transform{};
  mutable gl::UniformBuffer<Transform> m_transformBuffer;

  std::vector<std::tuple<glm::vec2, glm::vec2>> m_scissors;

  int m_renderOrder = 0;

  friend void setParent(gslu::nn_shared<Node> node, const std::shared_ptr<Node>& newParent);
  friend void setParent(Node* node, const std::shared_ptr<Node>& newParent);
};

inline void setParent(gslu::nn_shared<Node> node, // NOLINT(performance-unnecessary-value-param)
                      const std::shared_ptr<Node>& newParent)
{
  // first remove from hierarchy
  if(const auto currentParent = node->m_parent.lock())
  {
    if(currentParent == newParent)
      return;

    const auto it = std::find(currentParent->m_children.begin(), currentParent->m_children.end(), node);
    BOOST_ASSERT(it != currentParent->m_children.end());
    node->m_parent.reset();
    currentParent->m_children.erase(it);
  }

  // then add to hierarchy again
  node->m_parent = newParent;

  if(newParent != nullptr)
    newParent->m_children.push_back(node);

  node->transformChanged();
}

inline void setParent(Node* node, const std::shared_ptr<Node>& newParent)
{
  if(const auto currentParent = node->m_parent.lock())
  {
    if(currentParent == newParent)
      return;

    const auto sharedNode = currentParent->findChild(node);
    setParent(gsl::not_null{sharedNode}, newParent);
    return;
  }

  BOOST_THROW_EXCEPTION(std::runtime_error("Cannot initially assign parents to raw node pointers"));
}

inline void addChild(const gslu::nn_shared<Node>& node, const gslu::nn_shared<Node>& child)
{
  if(child->getParent().lock() == node.get())
  {
    // This node is already present in our hierarchy
    return;
  }

  setParent(child, node);
}
} // namespace render::scene
