#include "uniformparameter.h"

#include "camera.h"
#include "mesh.h"
#include "node.h"
#include "shaderprogram.h"

#include <boost/log/trivial.hpp>
#include <gl/program.h>

namespace render::scene
{
bool UniformParameter::bind(const Node& node,
                            const Mesh& mesh,
                            const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram)
{
  auto setter = mesh.findUniformSetter(getName());
  if(!m_valueSetter && setter == nullptr)
  {
    setter = node.findUniformSetter(getName());
    if(!m_valueSetter && setter == nullptr)
    {
      // don't have an explicit setter present on material, node or mesh level, assuming it's set on shader level
      return true;
    }
  }

  const auto uniform = findUniform(shaderProgram);
  if(uniform == nullptr)
    return false;

  if(setter != nullptr)
    (*setter)(node, mesh, *uniform);
  else
    m_valueSetter(node, mesh, *uniform);

  return true;
}

gl::Uniform* UniformParameter::findUniform(const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) const
{
  if(const auto uniform = shaderProgram->findUniform(getName()))
    return uniform;

  BOOST_LOG_TRIVIAL(warning) << "Uniform '" << getName() << "' not found in program '" << shaderProgram->getId() << "'";

  return nullptr;
}

bool UniformBlockParameter::bind(const Node& node,
                                 const Mesh& mesh,
                                 const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram)
{
  auto binder = mesh.findUniformBlockBinder(getName());
  if(!m_bufferBinder && binder == nullptr)
  {
    binder = node.findUniformBlockBinder(getName());
    if(!m_bufferBinder && binder == nullptr)
    {
      // don't have an explicit binder present on material, node or mesh level, assuming it's set on shader level
      return true;
    }
  }

  const auto block = findUniformBlock(shaderProgram);
  if(block == nullptr)
    return false;

  if(binder != nullptr)
    (*binder)(node, mesh, *block);
  else
    m_bufferBinder(node, mesh, *block);

  return true;
}

void UniformBlockParameter::bindTransformBuffer()
{
  m_bufferBinder = [](const Node& node, const Mesh& /*mesh*/, gl::UniformBlock& ub)
  {
    ub.bind(node.getTransformBuffer());
  };
}

void UniformBlockParameter::bindCameraBuffer(const gsl::not_null<std::shared_ptr<Camera>>& camera)
{
  m_bufferBinder = [camera](const Node& /*node*/, const Mesh& /*mesh*/, gl::UniformBlock& ub)
  {
    ub.bind(camera->getMatricesBuffer());
  };
}

gl::UniformBlock*
  UniformBlockParameter::findUniformBlock(const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) const
{
  if(const auto block = shaderProgram->findUniformBlock(getName()))
    return block;

  BOOST_LOG_TRIVIAL(warning) << "Uniform block '" << getName() << "' not found in program '" << shaderProgram->getId()
                             << "'";

  return nullptr;
}
} // namespace render::scene
