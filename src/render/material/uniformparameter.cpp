#include "uniformparameter.h"

#include "render/scene/camera.h"
#include "render/scene/mesh.h"
#include "render/scene/node.h"
#include "shaderprogram.h"

#include <boost/log/trivial.hpp>
#include <gl/program.h>

namespace render::material
{
bool UniformParameter::bind(const scene::Node* node, const scene::Mesh& mesh, ShaderProgram& shaderProgram)
{
  const auto* setter = m_valueSetter ? &m_valueSetter : nullptr;
  if(setter == nullptr)
  {
    setter = mesh.findUniformSetter(getName());
  }

  if(setter == nullptr && node != nullptr)
  {
    setter = node->findUniformSetter(getName());
  }

  if(setter == nullptr)
  {
    // don't have an explicit setter present on material, node or mesh level, assuming it's set on shader level
    return true;
  }

  const auto uniform = findUniform(shaderProgram);
  if(uniform == nullptr)
    return false;

  (*setter)(node, mesh, *uniform);

  return true;
}

gl::Uniform* UniformParameter::findUniform(ShaderProgram& shaderProgram) const
{
  if(const auto uniform = shaderProgram.findUniform(getName()))
    return uniform;

  BOOST_LOG_TRIVIAL(warning) << "Uniform '" << getName() << "' not found in program '" << shaderProgram.getId() << "'";

  return nullptr;
}

bool UniformBlockParameter::bind(const scene::Node* node, const scene::Mesh& mesh, ShaderProgram& shaderProgram)
{
  const auto* binder = m_bufferBinder ? &m_bufferBinder : nullptr;
  if(binder == nullptr)
  {
    binder = mesh.findUniformBlockBinder(getName());
  }

  if(binder == nullptr && node != nullptr)
  {
    binder = node->findUniformBlockBinder(getName());
  }

  if(binder == nullptr)
  {
    // don't have an explicit setter present on material, node or mesh level, assuming it's set on shader level
    return true;
  }

  const auto block = findUniformBlock(shaderProgram);
  if(block == nullptr)
    return false;

  (*binder)(node, mesh, *block);

  return true;
}

void UniformBlockParameter::bindTransformBuffer()
{
  m_bufferBinder = [](const scene::Node* node, const scene::Mesh& /*mesh*/, gl::UniformBlock& ub)
  {
    gsl_Expects(node != nullptr);
    ub.bind(node->getTransformBuffer());
  };
}

void UniformBlockParameter::bindCameraBuffer(const gslu::nn_shared<scene::Camera>& camera)
{
  m_bufferBinder = [camera](const scene::Node* /*node*/, const scene::Mesh& /*mesh*/, gl::UniformBlock& ub)
  {
    ub.bind(camera->getMatricesBuffer());
  };
}

gl::UniformBlock* UniformBlockParameter::findUniformBlock(ShaderProgram& shaderProgram) const
{
  if(const auto block = shaderProgram.findUniformBlock(getName()))
    return block;

  BOOST_LOG_TRIVIAL(warning) << "Uniform block '" << getName() << "' not found in program '" << shaderProgram.getId()
                             << "'";

  return nullptr;
}
} // namespace render::material
