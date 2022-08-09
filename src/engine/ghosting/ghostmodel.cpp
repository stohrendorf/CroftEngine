#include "ghostmodel.h"

#include "engine/engine.h"
#include "engine/engineconfig.h"
#include "engine/presenter.h"
#include "engine/world/rendermeshdata.h"
#include "engine/world/world.h"
#include "ghost.h"
#include "render/scene/materialgroup.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/rendermode.h"

#include <gl/buffer.h>
#include <gl/renderstate.h>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <vector>

namespace engine::ghosting
{
void GhostModel::apply(const world::World& world, const GhostFrame& frame)
{
  setLocalMatrix(frame.modelMatrix);
  m_roomId = frame.roomId;

  engine::world::RenderMeshDataCompositor compositor;
  std::vector<glm::mat4> matrices;
  for(const auto& bone : frame.bones)
  {
    matrices.emplace_back(bone.matrix);
    if(!bone.visible)
      compositor.appendEmpty();
    else
      compositor.append(*world.getMeshes().at(bone.meshIdx).meshData, gl::SRGBA8{0, 0, 0, 0});
  }

  if(compositor.empty())
  {
    setRenderable(nullptr);
  }
  else
  {
    auto mesh = compositor.toMesh(
      *world.getPresenter().getMaterialManager(),
      true,
      false,
      [&engine = world.getEngine()]()
      {
        return engine.getEngineConfig()->animSmoothing;
      },
      getName());
    mesh->getMaterialGroup().set(render::scene::RenderMode::Full,
                                 world.getPresenter().getMaterialManager()->getGhost(
                                   [&engine = world.getEngine()]()
                                   {
                                     return engine.getEngineConfig()->animSmoothing;
                                   }));
    mesh->getMaterialGroup().set(render::scene::RenderMode::DepthOnly, nullptr);
    mesh->getRenderState().setDepthWrite(false);
    mesh->getRenderState().setScissorTest(false);
    setRenderable(mesh);
  }

  if(m_meshMatricesBuffer == nullptr || m_meshMatricesBuffer->size() != matrices.size())
  {
    m_meshMatricesBuffer = std::make_unique<gl::ShaderStorageBuffer<glm::mat4>>(
      "mesh-matrices-ssb", gl::api::BufferUsage::DynamicDraw, matrices);
  }
  else
  {
    m_meshMatricesBuffer->setSubData(matrices, 0);
  }
}
} // namespace engine::ghosting
