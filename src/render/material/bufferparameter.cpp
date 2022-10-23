#include "bufferparameter.h"

#include "engine/ghosting/ghostmodel.h"
#include "engine/skeletalmodelnode.h"
#include "render/scene/mesh.h"
#include "render/scene/node.h"
#include "shaderprogram.h"

#include <boost/log/trivial.hpp>
#include <gl/program.h>
#include <iosfwd>

namespace render::material
{
bool BufferParameter::bind(const scene::Node* node,
                           const scene::Mesh& mesh,
                           const gslu::nn_shared<ShaderProgram>& shaderProgram)
{
  auto binder = mesh.findShaderStorageBlockBinder(getName());
  if(!m_bufferBinder && binder == nullptr)
  {
    if(node != nullptr)
      binder = node->findShaderStorageBlockBinder(getName());
    if(binder == nullptr)
    {
      // don't have an explicit binder present on material, node or mesh level, assuming it's set on shader level
      return true;
    }
  }

  const auto block = findShaderStorageBlock(shaderProgram);
  if(block == nullptr)
    return false;

  if(binder != nullptr)
    (*binder)(node, mesh, *block);
  else
    m_bufferBinder(node, mesh, *block);

  return true;
}

void BufferParameter::bindBoneTransformBuffer(std::function<bool()> smooth)
{
  m_bufferBinder = [smooth](const scene::Node* node, const scene::Mesh& /*mesh*/, gl::ShaderStorageBlock& ssb)
  {
    if(const auto* mo = dynamic_cast<const engine::SkeletalModelNode*>(node))
      ssb.bind(mo->getMeshMatricesBuffer(smooth));
    else if(const auto* go = dynamic_cast<const engine::ghosting::GhostModel*>(node))
      ssb.bind(go->getMeshMatricesBuffer());
  };
}

gl::ShaderStorageBlock*
  BufferParameter::findShaderStorageBlock(const gslu::nn_shared<ShaderProgram>& shaderProgram) const
{
  if(const auto block = shaderProgram->findShaderStorageBlock(getName()))
    return block;

  BOOST_LOG_TRIVIAL(warning) << "Shader storage block '" << getName() << "' not found in program '"
                             << shaderProgram->getId() << "'";

  return nullptr;
}
} // namespace render::material
