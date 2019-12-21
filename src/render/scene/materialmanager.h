#pragma once

#include "material.h"
#include "renderer.h"
#include "shadermanager.h"

namespace render::scene
{
class MaterialManager final
{
public:
  explicit MaterialManager(std::filesystem::path root)
      : m_shaderManager{std::move(root)}
  {
  }

  [[nodiscard]] auto& getShaderManager() const
  {
    return m_shaderManager;
  }

  [[nodiscard]] auto& getShaderManager()
  {
    return m_shaderManager;
  }

  [[nodiscard]] const auto& getSprite()
  {
    if(m_sprite != nullptr)
      return m_sprite;

    m_sprite = std::make_shared<Material>(m_shaderManager.getTextured());
    m_sprite->getRenderState().setCullFace(false);

    m_sprite->getUniform("u_modelMatrix")->bindModelMatrix();
    m_sprite->getUniform("u_camProjection")->bindProjectionMatrix();

    return m_sprite;
  }

  [[nodiscard]] const auto& getDepthOnly()
  {
    if(m_depthOnly != nullptr)
      return m_depthOnly;

    m_depthOnly = std::make_shared<Material>(m_shaderManager.getDepthOnly());
    m_depthOnly->getUniform("u_mvp")->bindLightModelViewProjection();

    return m_depthOnly;
  }

  [[nodiscard]] auto createTextureMaterial(const gsl::not_null<std::shared_ptr<render::gl::Texture>>& texture,
                                           const gsl::not_null<std::shared_ptr<render::gl::TextureDepth>>& lightDepth,
                                           bool water,
                                           const gsl::not_null<const Renderer*>& renderer)
  {
    auto result
      = std::make_shared<Material>(water ? m_shaderManager.getTexturedWater() : m_shaderManager.getTextured());
    texture->set(::gl::TextureParameterName::TextureWrapS, ::gl::TextureWrapMode::ClampToEdge);
    texture->set(::gl::TextureParameterName::TextureWrapT, ::gl::TextureWrapMode::ClampToEdge);
    result->getUniform("u_diffuseTexture")->set(texture.get());
    result->getUniform("u_modelMatrix")->bindModelMatrix();
    result->getUniform("u_modelViewMatrix")->bindModelViewMatrix();
    result->getUniform("u_camProjection")->bindProjectionMatrix();
    result->getUniform("u_lightMVP")->bindLightModelViewProjection();
    result->getUniform("u_lightDepth")->set(lightDepth.get());

    if(water)
    {
      result->getUniform("u_time")->bind([renderer](const Node&, render::gl::ProgramUniform& uniform) {
        const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
        uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
      });
    }

    return result;
  }

  [[nodiscard]] const auto& getColor(const gsl::not_null<std::shared_ptr<render::gl::TextureDepth>>& lightDepth)
  {
    if(m_color != nullptr)
      return m_color;

    m_color = std::make_shared<Material>(m_shaderManager.getColored());
    m_color->getUniform("u_modelMatrix")->bindModelMatrix();
    m_color->getUniform("u_modelViewMatrix")->bindModelViewMatrix();
    m_color->getUniform("u_camProjection")->bindProjectionMatrix();
    m_color->getUniform("u_lightMVP")->bindLightModelViewProjection();
    m_color->getUniform("u_lightDepth")->set(lightDepth.get());

    return m_color;
  }

  [[nodiscard]] const auto& getPortal(const gsl::not_null<const Renderer*>& renderer)
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

  [[nodiscard]] const auto& getLightning()
  {
    if(m_lightning != nullptr)
      return m_lightning;

    m_lightning = std::make_shared<render::scene::Material>(m_shaderManager.getLightning());
    m_lightning->getUniform("u_modelViewMatrix")->bindModelViewMatrix();
    m_lightning->getUniform("u_camProjection")->bindProjectionMatrix();

    return m_lightning;
  }

private:
  ShaderManager m_shaderManager;

  std::shared_ptr<Material> m_sprite{nullptr};
  std::shared_ptr<Material> m_depthOnly{nullptr};
  std::shared_ptr<Material> m_color{nullptr};
  std::shared_ptr<Material> m_portal{nullptr};
  std::shared_ptr<Material> m_lightning{nullptr};
};
} // namespace render::scene
