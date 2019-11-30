#pragma once

#include "gsl-lite.hpp"
#include "render/gl/renderstate.h"
#include "shaderprogram.h"

namespace render::scene
{
class UniformParameter;
class BufferParameter;

class Node;

class Material final
{
public:
  explicit Material(gsl::not_null<std::shared_ptr<ShaderProgram>> shaderProgram);

  Material(const Material&) = delete;

  Material(Material&&) = delete;

  Material& operator=(const Material&) = delete;

  Material& operator=(Material&&) = delete;

  ~Material();

  explicit Material(const std::string& vshPath,
                    const std::string& fshPath,
                    const std::vector<std::string>& defines = {});

  const gsl::not_null<std::shared_ptr<ShaderProgram>>& getShaderProgram() const
  {
    return m_shaderProgram;
  }

  void bind(const Node& node) const;

  gsl::not_null<std::shared_ptr<UniformParameter>> getUniform(const std::string& name) const;
  gsl::not_null<std::shared_ptr<BufferParameter>> getBuffer(const std::string& name) const;

  gl::RenderState& getRenderState()
  {
    return m_renderState;
  }

private:
  gsl::not_null<std::shared_ptr<ShaderProgram>> m_shaderProgram;

  mutable std::vector<gsl::not_null<std::shared_ptr<UniformParameter>>> m_uniforms;
  mutable std::vector<gsl::not_null<std::shared_ptr<BufferParameter>>> m_buffers;

  gl::RenderState m_renderState{};
};
} // namespace render::scene
