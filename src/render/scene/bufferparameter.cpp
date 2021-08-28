#include "bufferparameter.h"

#include "engine/skeletalmodelnode.h"
#include "mesh.h"
#include "node.h"

namespace render::scene
{
bool BufferParameter::bind(const Node& node,
                           const Mesh& mesh,
                           const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram)
{
  auto binder = mesh.findShaderStorageBlockBinder(getName());
  if(!m_bufferBinder && binder == nullptr)
  {
    binder = node.findShaderStorageBlockBinder(getName());
    if(!m_bufferBinder && binder == nullptr)
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

void BufferParameter::bindBoneTransformBuffer()
{
  m_bufferBinder = [](const Node& node, const Mesh& /*mesh*/, gl::ShaderStorageBlock& ssb)
  {
    if(const auto* mo = dynamic_cast<const engine::SkeletalModelNode*>(&node))
      ssb.bind(mo->getMeshMatricesBuffer());
  };
}

gl::ShaderStorageBlock*
  BufferParameter::findShaderStorageBlock(const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) const
{
  if(const auto block = shaderProgram->findShaderStorageBlock(getName()))
    return block;

  BOOST_LOG_TRIVIAL(warning) << "Shader storage block '" << getName() << "' not found in program '"
                             << shaderProgram->getId() << "'";

  return nullptr;
}
} // namespace render::scene
