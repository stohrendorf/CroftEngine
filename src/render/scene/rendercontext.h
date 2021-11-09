#pragma once

#include "node.h"
#include "rendermode.h"

#include <gl/renderstate.h>
#include <glm/glm.hpp>
#include <gsl/gsl-lite.hpp>
#include <optional>
#include <stack>

namespace render::scene
{
class RenderContext final
{
public:
  explicit RenderContext(RenderMode renderMode, const std::optional<glm::mat4>& viewProjection)
      : m_renderMode{renderMode}
      , m_viewProjection{viewProjection}
  {
    m_renderStates.push(gl::RenderState::getWantedState());
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
    gl::RenderState::getWantedState() = m_renderStates.top();
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

  [[nodiscard]] const auto& getCurrentState() const
  {
    Expects(!m_renderStates.empty());
    return m_renderStates.top();
  }

private:
  std::stack<gl::RenderState> m_renderStates{};
  const RenderMode m_renderMode;
  const std::optional<glm::mat4> m_viewProjection;
};
} // namespace render::scene
