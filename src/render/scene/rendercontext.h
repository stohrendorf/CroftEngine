#pragma once

#include "node.h"
#include "render/material/rendermode.h"

#include <gl/renderstate.h>
#include <glm/glm.hpp>
#include <gsl/gsl-lite.hpp>
#include <optional>
#include <stack>

namespace render::scene
{
enum class Translucency;

class RenderContext final
{
public:
  explicit RenderContext(material::RenderMode renderMode,
                         const std::optional<glm::mat4>& viewProjection,
                         Translucency translucencySelector)
      : m_renderMode{renderMode}
      , m_viewProjection{viewProjection}
      , m_translucencySelector{translucencySelector}
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
    gsl_Expects(!m_renderStates.empty());
    gl::RenderState::getWantedState() = m_renderStates.top();
  }

  void popState()
  {
    m_renderStates.pop();
  }

  [[nodiscard]] material::RenderMode getRenderMode() const noexcept
  {
    return m_renderMode;
  }

  [[nodiscard]] const auto& getViewProjection() const noexcept
  {
    return m_viewProjection;
  }

  [[nodiscard]] const auto& getCurrentState() const
  {
    gsl_Expects(!m_renderStates.empty());
    return m_renderStates.top();
  }

  [[nodiscard]] auto getTranslucencySelector() const
  {
    return m_translucencySelector;
  }

private:
  std::stack<gl::RenderState> m_renderStates{};
  const material::RenderMode m_renderMode;
  const std::optional<glm::mat4> m_viewProjection;
  const Translucency m_translucencySelector;
};
} // namespace render::scene
