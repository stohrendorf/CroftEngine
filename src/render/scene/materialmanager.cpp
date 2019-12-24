#include "materialmanager.h"

#include "csm.h"
#include "node.h"

namespace render::scene
{
const std::shared_ptr<Material>& MaterialManager::getSprite()
{
  if(m_sprite != nullptr)
    return m_sprite;

  m_sprite = std::make_shared<Material>(m_shaderManager.getTextured());
  m_sprite->getRenderState().setCullFace(false);

  m_sprite->getUniform("u_modelMatrix")->bindModelMatrix();
  m_sprite->getUniform("u_camProjection")->bindProjectionMatrix();

  return m_sprite;
}

const std::shared_ptr<Material>& MaterialManager::getDepthOnly()
{
  if(m_depthOnly != nullptr)
    return m_depthOnly;

  m_depthOnly = std::make_shared<Material>(m_shaderManager.getDepthOnly());
  m_depthOnly->getUniform("u_mvp")->bind([this](const Node& node, gl::ProgramUniform& uniform) {
    uniform.set(m_csm->getActiveMatrix(node.getModelMatrix()));
  });
  m_depthOnly->getRenderState().setDepthTest(true);
  m_depthOnly->getRenderState().setDepthWrite(true);

  return m_depthOnly;
}

std::shared_ptr<Material>
  MaterialManager::createTextureMaterial(const gsl::not_null<std::shared_ptr<render::gl::Texture>>& texture,
                                         bool water,
                                         const gsl::not_null<const Renderer*>& renderer)
{
  auto result = std::make_shared<Material>(water ? m_shaderManager.getTexturedWater() : m_shaderManager.getTextured());
  texture->set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::ClampToEdge);
  texture->set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::ClampToEdge);
  result->getUniform("u_diffuseTexture")->set(texture.get());
  result->getUniform("u_modelMatrix")->bindModelMatrix();
  result->getUniform("u_modelViewMatrix")->bindModelViewMatrix();
  result->getUniform("u_camProjection")->bindProjectionMatrix();
  result->getUniform("u_lightMVP[0]")->bind([this](const Node& node, gl::ProgramUniform& uniform) {
    uniform.set(m_csm->getMatrices(node.getModelMatrix()));
  });
  result->getUniform("u_csmSplits[0]")->bind([this](const Node&, gl::ProgramUniform& uniform) {
    uniform.set(m_csm->getSplitEnds());
  });
  result->getUniform("u_lightDepth[0]")->set(m_csm->getTextures());

  if(water)
  {
    result->getUniform("u_time")->bind([renderer](const Node&, render::gl::ProgramUniform& uniform) {
      const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
      uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
    });
  }

  return result;
}

const std::shared_ptr<Material>& MaterialManager::getColor()
{
  if(m_color != nullptr)
    return m_color;

  m_color = std::make_shared<Material>(m_shaderManager.getColored());
  m_color->getUniform("u_modelMatrix")->bindModelMatrix();
  m_color->getUniform("u_modelViewMatrix")->bindModelViewMatrix();
  m_color->getUniform("u_camProjection")->bindProjectionMatrix();
  m_color->getUniform("u_lightMVP[0]")->bind([this](const Node& node, gl::ProgramUniform& uniform) {
    uniform.set(m_csm->getMatrices(node.getModelMatrix()));
  });
  m_color->getUniform("u_csmSplits[0]")->bind([this](const Node&, gl::ProgramUniform& uniform) {
    uniform.set(m_csm->getSplitEnds());
  });
  m_color->getUniform("u_lightDepth[0]")->set(m_csm->getTextures());

  return m_color;
}

const std::shared_ptr<Material>& MaterialManager::getPortal(const gsl::not_null<const Renderer*>& renderer)
{
  if(m_portal != nullptr)
    return m_color;

  m_portal = std::make_shared<Material>(m_shaderManager.getPortal());
  m_portal->getRenderState().setCullFace(false);

  m_portal->getUniform("u_mvp")->bindViewProjectionMatrix(); // portals are in world space, no model transform needed
  m_portal->getUniform("u_time")->bind([renderer](const Node&, render::gl::ProgramUniform& uniform) {
    const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
    uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
  });

  return m_portal;
}

const std::shared_ptr<Material>& MaterialManager::getLightning()
{
  if(m_lightning != nullptr)
    return m_lightning;

  m_lightning = std::make_shared<render::scene::Material>(m_shaderManager.getLightning());
  m_lightning->getUniform("u_modelViewMatrix")->bindModelViewMatrix();
  m_lightning->getUniform("u_camProjection")->bindProjectionMatrix();

  return m_lightning;
}
} // namespace render::scene
