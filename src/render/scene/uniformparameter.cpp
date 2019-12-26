#include "uniformparameter.h"

#include "camera.h"
#include "node.h"
#include "scene.h"

namespace render::scene
{
bool UniformParameter::bind(const Node& node, const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram)
{
  const auto setter = node.findUniformSetter(getName());
  if(!m_valueSetter && setter == nullptr)
  {
    // don't have an explicit setter present on material or node level, assuming it's set on shader level
    return true;
  }

  const auto uniform = findUniform(shaderProgram);
  if(uniform == nullptr)
    return false;

  if(setter != nullptr)
    (*setter)(node, *uniform);
  else
    (*m_valueSetter)(node, *uniform);

  return true;
}

void UniformParameter::bindModelMatrix()
{
  m_valueSetter = [](const Node& node, gl::Uniform& uniform) { uniform.set(node.getModelMatrix()); };
}

void UniformParameter::bindViewMatrix()
{
  m_valueSetter = [](const Node& node, gl::Uniform& uniform) { uniform.set(node.getViewMatrix()); };
}

void UniformParameter::bindModelViewMatrix()
{
  m_valueSetter = [](const Node& node, gl::Uniform& uniform) { uniform.set(node.getModelViewMatrix()); };
}

void UniformParameter::bindProjectionMatrix()
{
  m_valueSetter = [](const Node& node, gl::Uniform& uniform) { uniform.set(node.getProjectionMatrix()); };
}

void UniformParameter::bindViewProjectionMatrix()
{
  m_valueSetter = [](const Node& node, gl::Uniform& uniform) { uniform.set(node.getViewProjectionMatrix()); };
}

bool UniformBlockParameter::bind(const Node& node, const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram)
{
  const auto binder = node.findUniformBlockBinder(getName());
  if(!m_bufferBinder.has_value() && binder == nullptr)
  {
    // don't have an explicit binder present on material or node level, assuming it's set on shader level
    return true;
  }

  const auto block = findUniformBlock(shaderProgram);
  if(block == nullptr)
    return false;

  if(binder != nullptr)
    (*binder)(node, *block);
  else
    (*m_bufferBinder)(node, *block);

  return true;
}
} // namespace render::scene
