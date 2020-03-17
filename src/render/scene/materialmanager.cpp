#include "materialmanager.h"

#include "csm.h"
#include "node.h"

#include <utility>

namespace render::scene
{
const std::shared_ptr<Material>& MaterialManager::getSprite()
{
  if(m_sprite != nullptr)
    return m_sprite;

  m_sprite = std::make_shared<Material>(m_shaderManager->getGeometry());
  m_sprite->getRenderState().setCullFace(false);

  m_sprite->getUniformBlock("Transform")->bindTransformBuffer();
  m_sprite->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());

  return m_sprite;
}

const std::shared_ptr<Material>& MaterialManager::getCSMDepthOnly()
{
  if(m_csmDepthOnly != nullptr)
    return m_csmDepthOnly;

  m_csmDepthOnly = std::make_shared<Material>(m_shaderManager->getCSMDepthOnly());
  m_csmDepthOnly->getUniform("u_mvp")->bind(
    [this](const Node& node, gl::Uniform& uniform) { uniform.set(m_csm->getActiveMatrix(node.getModelMatrix())); });
  m_csmDepthOnly->getRenderState().setDepthTest(true);
  m_csmDepthOnly->getRenderState().setDepthWrite(true);

  return m_csmDepthOnly;
}

const std::shared_ptr<Material>& MaterialManager::getDepthOnly()
{
  if(m_depthOnly != nullptr)
    return m_depthOnly;

  m_depthOnly = std::make_shared<Material>(m_shaderManager->getDepthOnly());
  m_depthOnly->getRenderState().setDepthTest(true);
  m_depthOnly->getRenderState().setDepthWrite(true);
  m_depthOnly->getUniformBlock("Transform")->bindTransformBuffer();
  m_depthOnly->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());

  return m_depthOnly;
}

std::shared_ptr<Material>
  MaterialManager::createMaterial(const gsl::not_null<std::shared_ptr<gl::Texture2DArray<gl::SRGBA8>>>& texture,
                                  bool water)
{
  if(!water && m_material != nullptr)
    return m_material;
  else if(water && m_materialWater != nullptr)
    return m_materialWater;

  texture->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge);
  texture->set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge);

  auto m = std::make_shared<Material>(water ? m_shaderManager->getGeometryWater() : m_shaderManager->getGeometry());
  m->getUniform("u_diffuseTextures")->set(texture);
  m->getUniform("u_spritePole")->set(-1);

  m->getUniformBlock("Transform")->bindTransformBuffer();
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  m->getUniformBlock("CSM")->bind(
    [this](const Node& node, gl::UniformBlock& ub) { ub.bind(m_csm->getBuffer(node.getModelMatrix())); });

  m->getUniform("u_csmVsm[0]")->set(m_csm->getTextures());

  if(water)
  {
    m->getUniform("u_time")->bind([renderer = m_renderer](const Node&, gl::Uniform& uniform) {
      const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
      uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
    });
    m_materialWater = m;
  }
  else
    m_material = m;

  return m;
}

const std::shared_ptr<Material>& MaterialManager::getPortal()
{
  if(m_portal != nullptr)
    return m_portal;

  m_portal = std::make_shared<Material>(m_shaderManager->getPortal());
  m_portal->getRenderState().setCullFace(false);

  m_portal->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  m_portal->getUniform("u_time")->bind([renderer = m_renderer](const Node&, gl::Uniform& uniform) {
    const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
    uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
  });

  return m_portal;
}

const std::shared_ptr<Material>& MaterialManager::getLightning()
{
  if(m_lightning != nullptr)
    return m_lightning;

  m_lightning = std::make_shared<render::scene::Material>(m_shaderManager->getLightning());
  m_lightning->getUniformBlock("Transform")->bindTransformBuffer();
  m_lightning->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());

  return m_lightning;
}

MaterialManager::MaterialManager(gsl::not_null<std::shared_ptr<ShaderManager>> shaderManager,
                                 gsl::not_null<std::shared_ptr<CSM>> csm,
                                 gsl::not_null<std::shared_ptr<Renderer>> renderer)
    : m_shaderManager{std::move(shaderManager)}
    , m_csm{std::move(csm)}
    , m_renderer{std::move(renderer)}
{
}
} // namespace render::scene
