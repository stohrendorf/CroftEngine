#pragma once

#include "rendermode.h"

#include <gl/renderstate.h>
#include <glm/glm.hpp>
#include <gsl-lite.hpp>
#include <optional>
#include <stack>

namespace render::scene
{
class Node;
class RenderContext final
{
public:
  explicit RenderContext(RenderMode renderMode, const std::optional<glm::mat4>& viewProjection)
      : m_renderMode{renderMode}
      , m_viewProjection{viewProjection}
  {
    m_renderStates.push(gl::RenderState());
  }

  [[nodiscard]] Node* getCurrentNode() const noexcept
  {
    return m_currentNode;
  }

  void setCurrentNode(Node* n) noexcept
  {
    m_currentNode = n;
  }

  void pushState(const gl::RenderState& state)
  {
    auto tmp = m_renderStates.top();
    tmp.merge(state);
    m_renderStates.emplace(tmp);
  }

  void bindState()
  {
    Expects(!m_renderStates.empty());
    m_renderStates.top().apply();
  }

  void popState()
  {
    m_renderStates.pop();
  }

  [[nodiscard]] RenderMode getRenderMode() const noexcept
  {
    return m_renderMode;
  }

  [[nodiscard]] const auto& getViewProjection() const noexcept
  {
    return m_viewProjection;
  }

private:
  Node* m_currentNode = nullptr;
  std::stack<gl::RenderState> m_renderStates{};
  const RenderMode m_renderMode;
  const std::optional<glm::mat4> m_viewProjection;
};
} // namespace render::scene
