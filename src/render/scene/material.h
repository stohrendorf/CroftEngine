#pragma once

#include <gl/renderstate.h>
#include <gsl/gsl-lite.hpp>
#include <vector>

namespace render::scene
{
class BufferParameter;
class Mesh;
class Node;
class ShaderProgram;
class UniformBlockParameter;
class UniformParameter;

class Material final
{
public:
  explicit Material(gsl::not_null<std::shared_ptr<ShaderProgram>> shaderProgram);

  ~Material();

  Material(const Material&) = delete;
  Material(Material&&) = delete;
  Material& operator=(const Material&) = delete;
  Material& operator=(Material&&) = delete;

  const gsl::not_null<std::shared_ptr<ShaderProgram>>& getShaderProgram() const
  {
    return m_shaderProgram;
  }

  void bind(const Node& node, const Mesh& mesh) const;

  std::shared_ptr<UniformParameter> tryGetUniform(const std::string& name) const;
  gsl::not_null<std::shared_ptr<UniformParameter>> getUniform(const std::string& name) const
  {
    auto result = tryGetUniform(name);
    Expects(result != nullptr);
    return result;
  }

  std::shared_ptr<UniformBlockParameter> tryGetUniformBlock(const std::string& name) const;
  gsl::not_null<std::shared_ptr<UniformBlockParameter>> getUniformBlock(const std::string& name) const
  {
    auto result = tryGetUniformBlock(name);
    Expects(result != nullptr);
    return result;
  }

  std::shared_ptr<BufferParameter> tryGetBuffer(const std::string& name) const;
  gsl::not_null<std::shared_ptr<BufferParameter>> getBuffer(const std::string& name) const
  {
    auto result = tryGetBuffer(name);
    Expects(result != nullptr);
    return result;
  }

  gl::RenderState& getRenderState()
  {
    return m_renderState;
  }

private:
  gsl::not_null<std::shared_ptr<ShaderProgram>> m_shaderProgram;

  mutable std::vector<gsl::not_null<std::shared_ptr<UniformParameter>>> m_uniforms;
  mutable std::vector<gsl::not_null<std::shared_ptr<UniformBlockParameter>>> m_uniformBlocks;
  mutable std::vector<gsl::not_null<std::shared_ptr<BufferParameter>>> m_buffers;

  gl::RenderState m_renderState{};
};
} // namespace render::scene
