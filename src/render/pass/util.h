#pragma once

#include "render/scene/node.h"

namespace render::pass
{
inline std::shared_ptr<scene::Mesh> createFbMesh(const glm::ivec2& size, const gl::Program& program)
{
  auto mesh = scene::createQuadFullscreen(gsl::narrow<float>(size.x), gsl::narrow<float>(size.y), program);
  mesh->getRenderState().setCullFace(false);
  mesh->getRenderState().setDepthTest(false);
  mesh->getRenderState().setDepthWrite(false);
  return mesh;
}
} // namespace render::pass
