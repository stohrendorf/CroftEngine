#pragma once

#include "gsl-lite.hpp"
#include "render/gl/renderstate.h"

#include <stack>

namespace render::scene
{
class Node;

enum class RenderMode
{
  Full,
  DepthOnly
};

class RenderContext final
{
public:
  explicit RenderContext(RenderMode renderMode)
      : m_renderMode{renderMode}
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

  RenderMode getRenderMode() const noexcept
  {
    return m_renderMode;
  }

private:
  Node* m_currentNode = nullptr;
  std::stack<gl::RenderState> m_renderStates{};
  const RenderMode m_renderMode;
};
} // namespace render::scene
