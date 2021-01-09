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
    m_valueSetter(node, *uniform);

  return true;
}

bool UniformBlockParameter::bind(const Node& node, const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram)
{
  const auto binder = node.findUniformBlockBinder(getName());
  if(!m_bufferBinder && binder == nullptr)
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
    m_bufferBinder(node, *block);

  return true;
}

void UniformBlockParameter::bindTransformBuffer()
{
  m_bufferBinder = [](const Node& node, gl::UniformBlock& ub) { ub.bind(node.getTransformBuffer()); };
}

void UniformBlockParameter::bindCameraBuffer(const gsl::not_null<std::shared_ptr<Camera>>& camera)
{
  m_bufferBinder = [camera](const Node& /*node*/, gl::UniformBlock& ub) { ub.bind(camera->getMatricesBuffer()); };
}
} // namespace render::scene
