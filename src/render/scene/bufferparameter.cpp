#include "bufferparameter.h"

#include "engine/skeletalmodelnode.h"
#include "node.h"

namespace render::scene
{
bool BufferParameter::bind(const Node& node, const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram)
{
  const auto binder = node.findShaderStorageBlockBinder(getName());
  if(!m_bufferBinder.has_value() && binder == nullptr)
  {
    // don't have an explicit binder present on material or node level, assuming it's set on shader level
    return true;
  }

  const auto block = findShaderStorageBlock(shaderProgram);
  if(block == nullptr)
    return false;

  if(binder != nullptr)
    (*binder)(node, *block);
  else
    (*m_bufferBinder)(node, *block);

  return true;
}

void BufferParameter::bindBoneTransformBuffer()
{
  m_bufferBinder = [](const Node& node, gl::ShaderStorageBlock& ssb) {
    if(const auto* mo = dynamic_cast<const engine::SkeletalModelNode*>(&node))
      ssb.bind(mo->getMeshMatricesBuffer());
  };
}
} // namespace render::scene
