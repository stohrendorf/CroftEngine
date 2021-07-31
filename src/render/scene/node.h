#pragma once

#include "materialparameteroverrider.h"
#include "visitor.h"

namespace render::scene
{
class Renderable;

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

  using List = std::vector<gsl::not_null<std::shared_ptr<Node>>>;

  explicit Node(std::string name)
      : m_name{std::move(name)}
      , m_transformBuffer{m_name + "-transform-ubo"}
  {
  }

  ~Node() override;

  const std::string& getName() const
  {
    return m_name;
  }

  const std::weak_ptr<Node>& getParent() const
  {
    return m_parent;
  }

  void setVisible(bool visible)
  {
    m_visible = visible;
  }

  bool isVisible() const
  {
    return m_visible;
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  const glm::mat4& getModelMatrix() const
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

  glm::vec3 getTranslationWorld() const
  {
    return glm::vec3(getModelMatrix()[3]);
  }

  const std::shared_ptr<Renderable>& getRenderable() const
  {
    return m_renderable;
  }

  void setRenderable(const std::shared_ptr<Renderable>& renderable)
  {
    m_renderable = renderable;
  }

  const List& getChildren() const
  {
    return m_children;
  }

  List& getChildren()
  {
    return m_children;
  }

  void removeAllChildren()
  {
    for(auto& child : m_children)
      child->m_parent.reset();
    m_children.clear();
  }

  const glm::mat4& getLocalMatrix() const
  {
    return m_localMatrix;
  }

  void setLocalMatrix(const glm::mat4& m)
  {
    m_localMatrix = m;
    transformChanged();
  }

  void accept(Visitor& visitor);

  std::shared_ptr<Node> findChild(const Node* node) const
  {
    const auto it
      = std::find_if(m_children.begin(),
                     m_children.end(),
                     [node](const gsl::not_null<std::shared_ptr<Node>>& ptr) { return ptr.get().get() == node; });

    if(it == m_children.end())
      return nullptr;

    return *it;
  }

  [[nodiscard]] const auto& getTransformBuffer() const
  {
    getModelMatrix(); // update data if dirty
    if(!m_bufferDirty)
      return m_transformBuffer;

    m_bufferDirty = false;
    m_transformBuffer.setData(m_transform, ::gl::api::BufferUsage::StreamDraw);
    return m_transformBuffer;
  }

  virtual bool canBeCulled(const glm::mat4& /*viewProjection*/) const
  {
    return false;
  }

  void clear()
  {
    auto tmp = m_children;
    for(const auto& node : tmp)
      setParent(node, nullptr);
  }

private:
  void transformChanged();

  std::string m_name;
  List m_children;
  std::weak_ptr<Node> m_parent{};
  bool m_visible = true;
  std::shared_ptr<Renderable> m_renderable = nullptr;
  glm::mat4 m_localMatrix{1.0f};

  mutable bool m_dirty = false;
  mutable bool m_bufferDirty = true;
  mutable Transform m_transform{};
  mutable gl::UniformBuffer<Transform> m_transformBuffer;

  friend void setParent(gsl::not_null<std::shared_ptr<Node>> node, const std::shared_ptr<Node>& newParent);
  friend void setParent(Node* node, const std::shared_ptr<Node>& newParent);
};

inline void setParent(gsl::not_null<std::shared_ptr<Node>> node, // NOLINT(performance-unnecessary-value-param)
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
    Expects(sharedNode != nullptr);
    setParent(sharedNode, newParent);
    return;
  }

  BOOST_THROW_EXCEPTION(std::runtime_error("Cannot initially assign parents to raw node pointers"));
}

inline void addChild(const gsl::not_null<std::shared_ptr<Node>>& node,
                     const gsl::not_null<std::shared_ptr<Node>>& child)
{
  if(child->getParent().lock() == node.get())
  {
    // This node is already present in our hierarchy
    return;
  }

  setParent(child, node);
}
} // namespace render::scene
