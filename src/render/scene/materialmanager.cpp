#include "materialmanager.h"

#include "csm.h"
#include "node.h"
#include "renderer.h"

#include <gl/texture2darray.h>
#include <utility>

namespace render::scene
{
const std::shared_ptr<Material>& MaterialManager::getSprite()
{
  if(m_sprite != nullptr)
    return m_sprite;

  m_sprite = std::make_shared<Material>(m_shaderManager->getGeometry(false, false, true));
  m_sprite->getRenderState().setCullFace(false);

  m_sprite->getUniformBlock("Transform")->bindTransformBuffer();
  m_sprite->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());

  return m_sprite;
}

const std::shared_ptr<Material>& MaterialManager::getCSMDepthOnly(bool skeletal)
{
  if(const auto& tmp = m_csmDepthOnly[skeletal])
    return tmp;

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
  if(const auto& tmp = m_depthOnly[skeletal])
    return tmp;

  m_depthOnly[skeletal] = std::make_shared<Material>(m_shaderManager->getDepthOnly(skeletal));
  m_depthOnly[skeletal]->getRenderState().setDepthTest(true);
  m_depthOnly[skeletal]->getRenderState().setDepthWrite(true);
  m_depthOnly[skeletal]->getUniformBlock("Transform")->bindTransformBuffer();
  m_depthOnly[skeletal]->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  if(skeletal)
    m_depthOnly[skeletal]->getBuffer("BoneTransform")->bindBoneTransformBuffer();
  m_depthOnly[skeletal]->getUniform("u_diffuseTextures")->set(m_geometryTextures);

  return m_depthOnly[skeletal];
}

std::shared_ptr<Material> MaterialManager::getGeometry(bool water, bool skeletal, bool roomShadowing)
{
  Expects(m_geometryTextures != nullptr);
  if(auto tmp = m_geometry[water][skeletal][roomShadowing])
    return tmp;

  auto m = std::make_shared<Material>(m_shaderManager->getGeometry(water, skeletal, roomShadowing));
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

  m_geometry[water][skeletal][roomShadowing] = m;
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

std::shared_ptr<Material> MaterialManager::getComposition(bool water, bool lensDistortion, bool dof, bool filmGrain)
{
  if(auto tmp = m_composition[water][lensDistortion][dof][filmGrain])
    return tmp;
  auto m = std::make_shared<Material>(m_shaderManager->getComposition(water, lensDistortion, dof, filmGrain));

  if(m->getShaderProgram()->findUniform("u_time") != nullptr)
    m->getUniform("u_time")->bind([renderer = m_renderer](const Node&, gl::Uniform& uniform) {
      const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
      uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
    });

  if(lensDistortion)
    m->getUniform("distortion_power")->set(water ? -2.0f : -1.0f);

  m_composition[water][lensDistortion][dof][filmGrain] = m;
  return m;
}

const std::shared_ptr<Material>& MaterialManager::getCrt()
{
  if(m_crt != nullptr)
    return m_crt;

  auto m = std::make_shared<Material>(m_shaderManager->getCrt());
  m->getUniform("u_time")->bind([renderer = m_renderer](const Node&, gl::Uniform& uniform) {
    const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
    uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
  });
  m_crt = m;
  return m_crt;
}

const std::shared_ptr<Material>& MaterialManager::getScreenSpriteTextured()
{
  if(m_screenSpriteTextured != nullptr)
    return m_screenSpriteTextured;

  auto m = std::make_shared<Material>(m_shaderManager->getScreenSpriteTextured());
  m->getUniform("u_input")->set(m_geometryTextures);
  m->getRenderState().setBlend(true);
  m->getRenderState().setBlendSrc(gl::api::BlendingFactor::SrcAlpha);
  m->getRenderState().setBlendDst(gl::api::BlendingFactor::OneMinusSrcAlpha);
  m->getRenderState().setDepthTest(false);
  m->getRenderState().setDepthWrite(false);
  m_screenSpriteTextured = m;
  return m_screenSpriteTextured;
}

const std::shared_ptr<Material>& MaterialManager::getScreenSpriteColorRect()
{
  if(m_screenSpriteColorRect != nullptr)
    return m_screenSpriteColorRect;

  auto m = std::make_shared<Material>(m_shaderManager->getScreenSpriteColorRect());
  m->getRenderState().setBlend(true);
  m->getRenderState().setBlendSrc(gl::api::BlendingFactor::SrcAlpha);
  m->getRenderState().setBlendDst(gl::api::BlendingFactor::OneMinusSrcAlpha);
  m->getRenderState().setDepthTest(false);
  m->getRenderState().setDepthWrite(false);
  m_screenSpriteColorRect = m;
  return m_screenSpriteColorRect;
}

void MaterialManager::setGeometryTextures(std::shared_ptr<gl::Texture2DArray<gl::SRGBA8>> geometryTextures)
{
  m_geometryTextures = std::move(geometryTextures);
  for(const auto& a : m_geometry)
    for(const auto& b : a)
      for(const auto& c : b)
        if(c != nullptr)
          c->getUniform("u_diffuseTextures")->set(m_geometryTextures);
  if(m_screenSpriteTextured != nullptr)
    m_screenSpriteTextured->getUniform("u_input")->set(m_geometryTextures);

  for(const auto& m : m_depthOnly)
    if(m != nullptr)
      m->getUniform("u_diffuseTextures")->set(m_geometryTextures);
}

void MaterialManager::setBilinearFiltering(bool enabled)
{
  if(m_geometryTextures == nullptr)
    return;

  if(enabled)
  {
    m_geometryTextures->set(gl::api::TextureMinFilter::LinearMipmapLinear);
    m_geometryTextures->set(gl::api::TextureMagFilter::Linear);
  }
  else
  {
    m_geometryTextures->set(gl::api::TextureMinFilter::NearestMipmapLinear);
    m_geometryTextures->set(gl::api::TextureMagFilter::Nearest);
  }
}
} // namespace render::scene
