#include "bufferparameter.h"

#include "engine/ghosting/ghostmodel.h"
#include "engine/skeletalmodelnode.h"
#include "render/scene/mesh.h"
#include "render/scene/node.h"
#include "shaderprogram.h"

#include <boost/log/trivial.hpp>
#include <functional>
#include <gl/program.h>

namespace render::material
{
bool BufferParameter::bind(const scene::Node* node, const scene::Mesh& mesh, ShaderProgram& shaderProgram)
{
  const auto* binder = m_bufferBinder ? &m_bufferBinder : nullptr;
  if(binder == nullptr)
  {
    binder = mesh.findShaderStorageBlockBinder(getName());
  }

  if(binder == nullptr && node != nullptr)
  {
    binder = node->findShaderStorageBlockBinder(getName());
  }

  if(binder == nullptr)
  {
    // don't have an explicit setter present on material, node or mesh level, assuming it's set on shader level
    return true;
  }

  const auto block = findShaderStorageBlock(shaderProgram);
  if(block == nullptr)
    return false;

  (*binder)(node, mesh, *block);

  return true;
}

void BufferParameter::bindBoneTransformBuffer(const std::function<bool()>& smooth)
{
  m_bufferBinder = [smooth](const scene::Node* node, const scene::Mesh& /*mesh*/, gl::ShaderStorageBlock& ssb)
  {
    if(const auto* mo = dynamic_cast<const engine::SkeletalModelNode*>(node))
      ssb.bind(mo->getMeshMatricesBuffer(smooth));
    else if(const auto* go = dynamic_cast<const engine::ghosting::GhostModel*>(node))
      ssb.bind(go->getMeshMatricesBuffer());
  };
}

gl::ShaderStorageBlock* BufferParameter::findShaderStorageBlock(ShaderProgram& shaderProgram) const
{
  if(const auto block = shaderProgram.findShaderStorageBlock(getName()))
    return block;

  BOOST_LOG_TRIVIAL(warning) << "Shader storage block '" << getName() << "' not found in program '"
                             << shaderProgram.getId() << "'";

  return nullptr;
}
} // namespace render::material
