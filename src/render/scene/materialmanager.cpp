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

  m_sprite = std::make_shared<Material>(m_shaderManager->getGeometry(false, false));
  m_sprite->getRenderState().setCullFace(false);

  m_sprite->getUniformBlock("Transform")->bindTransformBuffer();
  m_sprite->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());

  return m_sprite;
}

const std::shared_ptr<Material>& MaterialManager::getCSMDepthOnly(bool skeletal)
{
  if(const auto tmp = m_csmDepthOnly[skeletal])
    return m_csmDepthOnly[skeletal];

  m_csmDepthOnly[skeletal] = std::make_shared<Material>(m_shaderManager->getCSMDepthOnly(skeletal));
  m_csmDepthOnly[skeletal]->getUniform("u_mvp")->bind(
    [this](const Node& node, gl::Uniform& uniform) { uniform.set(m_csm->getActiveMatrix(node.getModelMatrix())); });
  m_csmDepthOnly[skeletal]->getRenderState().setDepthTest(true);
  m_csmDepthOnly[skeletal]->getRenderState().setDepthWrite(true);
  if(skeletal)
    m_csmDepthOnly[skeletal]->getBuffer("BoneTransform")->bindBoneTransformBuffer();

  return m_csmDepthOnly[skeletal];
}

const std::shared_ptr<Material>& MaterialManager::getDepthOnly(bool skeletal)
{
  if(const auto tmp = m_depthOnly[skeletal])
    return m_depthOnly[skeletal];

  m_depthOnly[skeletal] = std::make_shared<Material>(m_shaderManager->getDepthOnly(skeletal));
  m_depthOnly[skeletal]->getRenderState().setDepthTest(true);
  m_depthOnly[skeletal]->getRenderState().setDepthWrite(true);
  m_depthOnly[skeletal]->getUniformBlock("Transform")->bindTransformBuffer();
  m_depthOnly[skeletal]->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  if(skeletal)
    m_depthOnly[skeletal]->getBuffer("BoneTransform")->bindBoneTransformBuffer();

  return m_depthOnly[skeletal];
}

std::shared_ptr<Material> MaterialManager::getGeometry(bool water, bool skeletal)
{
  Expects(m_geometryTextures != nullptr);
  if(const auto tmp = m_geometry[water][skeletal])
    return tmp;

  auto m = std::make_shared<Material>(m_shaderManager->getGeometry(water, skeletal));
  m->getUniform("u_diffuseTextures")->set(m_geometryTextures);
  m->getUniform("u_spritePole")->set(-1);

  m->getUniformBlock("Transform")->bindTransformBuffer();
  if(skeletal)
    m->getBuffer("BoneTransform")->bindBoneTransformBuffer();
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
  }

  m_geometry[water][skeletal] = m;
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

std::shared_ptr<Material> MaterialManager::getComposition(bool water)
{
  if(const auto tmp = m_composition[water])
    return tmp;
  auto m = std::make_shared<Material>(m_shaderManager->getComposition(water));

  m->getUniform("u_time")->bind([renderer = m_renderer](const Node&, gl::Uniform& uniform) {
    const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
    uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
  });

  m->getUniform("distortion_power")->set(water ? -2.0f : -1.0f);

  m_composition[water] = m;
  return m;
}

const std::shared_ptr<Material>& MaterialManager::getVcr()
{
  if(m_vcr != nullptr)
    return m_vcr;

  auto m = std::make_shared<Material>(m_shaderManager->getVcr());
  m->getUniform("u_time")->bind([renderer = m_renderer](const Node&, gl::Uniform& uniform) {
    const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
    uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
  });
  m_vcr = m;
  return m_vcr;
}
} // namespace render::scene
