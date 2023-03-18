#pragma once

#include <gl/renderstate.h>
#include <gslu.h>
#include <memory>
#include <string>
#include <vector>

namespace render::scene
{
class Mesh;
class Node;
} // namespace render::scene

namespace render::material
{
class BufferParameter;
class ShaderProgram;
class UniformBlockParameter;
class UniformParameter;

class Material final
{
public:
  explicit Material(gslu::nn_shared<ShaderProgram> shaderProgram);

  ~Material();

  Material(const Material&) = delete;
  Material(Material&&) = delete;
  Material& operator=(const Material&) = delete;
  Material& operator=(Material&&) = delete;

  const gslu::nn_shared<ShaderProgram>& getShaderProgram() const
  {
    return m_shaderProgram;
  }

  void bind(const scene::Node* node, const scene::Mesh& mesh) const;

  std::shared_ptr<UniformParameter> tryGetUniform(const std::string& name) const;
  gslu::nn_shared<UniformParameter> getUniform(const std::string& name) const
  {
    return gsl::not_null{tryGetUniform(name)};
  }

  std::shared_ptr<UniformBlockParameter> tryGetUniformBlock(const std::string& name) const;
  gslu::nn_shared<UniformBlockParameter> getUniformBlock(const std::string& name) const
  {
    return gsl::not_null{tryGetUniformBlock(name)};
  }

  std::shared_ptr<BufferParameter> tryGetBuffer(const std::string& name) const;
  gslu::nn_shared<BufferParameter> getBuffer(const std::string& name) const
  {
    return gsl::not_null{tryGetBuffer(name)};
  }

  gl::RenderState& getRenderState()
  {
    return m_renderState;
  }

private:
  gslu::nn_shared<ShaderProgram> m_shaderProgram;

  mutable std::vector<gslu::nn_shared<UniformParameter>> m_uniforms;
  mutable std::vector<gslu::nn_shared<UniformBlockParameter>> m_uniformBlocks;
  mutable std::vector<gslu::nn_shared<BufferParameter>> m_buffers;

  gl::RenderState m_renderState{};
};
} // namespace render::material
