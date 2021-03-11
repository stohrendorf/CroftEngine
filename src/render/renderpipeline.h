#pragma once

#include "rendersettings.h"
#include "scene/blur.h"
#include "scene/camera.h"
#include "scene/material.h"
#include "scene/rendercontext.h"

#include <chrono>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <memory>

namespace render
{
namespace scene
{
class ShaderManager;
class MaterialManager;
} // namespace scene

class RenderPipeline
{
public:
  static constexpr bool FlushStages = false;

private:
  static std::shared_ptr<scene::Mesh> createFbMesh(const glm::ivec2& size, const gl::Program& program);

  class PortalStage
  {
  public:
    explicit PortalStage(scene::ShaderManager& shaderManager, const glm::vec2& viewport);

    void bind(const gl::TextureDepth<float>& depth);

    void renderBlur()
    {
      m_blur.render();
    }

    [[nodiscard]] const auto& getDepthBuffer() const
    {
      return m_depthBuffer;
    }

    [[nodiscard]] const auto& getNoisyTexture() const
    {
      return m_perturbBuffer;
    }

    [[nodiscard]] const auto& getBlurredTexture() const
    {
      return m_blur.getBlurredTexture();
    }

  private:
    std::shared_ptr<gl::TextureDepth<float>> m_depthBuffer;
    std::shared_ptr<gl::Texture2D<gl::RG32F>> m_perturbBuffer;
    scene::SeparableBlur<gl::RG32F> m_blur;
    std::shared_ptr<gl::Framebuffer> m_fb;
  };

  class GeometryStage
  {
  public:
    explicit GeometryStage(const glm::ivec2& viewport);
    void bind(const glm::ivec2& size);

    [[nodiscard]] const auto& getNormalBuffer() const
    {
      return m_normalBuffer;
    }

    [[nodiscard]] const auto& getPositionBuffer() const
    {
      return m_positionBuffer;
    }

    [[nodiscard]] const auto& getColorBuffer() const
    {
      return m_colorBuffer;
    }

    [[nodiscard]] const auto& getDepthBuffer() const
    {
      return m_depthBuffer;
    }

  private:
    std::shared_ptr<gl::TextureDepth<float>> m_depthBuffer;
    std::shared_ptr<gl::Texture2D<gl::SRGBA8>> m_colorBuffer;
    std::shared_ptr<gl::Texture2D<gl::RGB32F>> m_positionBuffer;
    std::shared_ptr<gl::Texture2D<gl::RGB16F>> m_normalBuffer;
    std::shared_ptr<gl::Framebuffer> m_fb;
  };

  class SSAOStage
  {
  public:
    explicit SSAOStage(scene::ShaderManager& shaderManager,
                       const glm::ivec2& viewport,
                       const GeometryStage& geometryStage);
    void updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera);

    void render(const glm::ivec2& size);

    [[nodiscard]] const auto& getBlurredTexture() const
    {
      return m_blur.getBlurredTexture();
    }

  private:
    const std::shared_ptr<scene::ShaderProgram> m_shader;
    const std::shared_ptr<scene::Material> m_material;

    std::shared_ptr<scene::Mesh> m_renderMesh;

    std::shared_ptr<gl::Texture2D<gl::RGB32F>> m_noiseTexture;
    std::shared_ptr<gl::Texture2D<gl::Scalar16F>> m_aoBuffer;
    std::shared_ptr<gl::Framebuffer> m_fb;

    scene::SeparableBlur<gl::Scalar16F> m_blur;
  };

  class FXAAStage
  {
  public:
    explicit FXAAStage(scene::ShaderManager& shaderManager,
                       const glm::ivec2& viewport,
                       const GeometryStage& geometryStage);

    void bind();

    void render(const glm::ivec2& size);

    [[nodiscard]] const auto& getColorBuffer() const
    {
      return m_colorBuffer;
    }

  private:
    const std::shared_ptr<scene::ShaderProgram> m_shader;
    const std::shared_ptr<scene::Material> m_material;
    std::shared_ptr<scene::Mesh> m_mesh;
    std::shared_ptr<gl::Texture2D<gl::SRGBA8>> m_colorBuffer;
    std::shared_ptr<gl::Framebuffer> m_fb;
  };

  class CompositionStage
  {
  public:
    explicit CompositionStage(scene::MaterialManager& materialManager,
                              const RenderSettings& renderSettings,
                              const glm::ivec2& viewport,
                              const GeometryStage& geometryStage,
                              const PortalStage& portalStage,
                              const SSAOStage& ssaoStage,
                              const FXAAStage& fxaaStage);

    void updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera);

    void render(bool water, const RenderSettings& renderSettings);

  private:
    std::shared_ptr<scene::Material> m_compositionMaterial;
    std::shared_ptr<scene::Material> m_waterCompositionMaterial;
    const std::shared_ptr<scene::Material> m_crtMaterial;

    std::shared_ptr<scene::Mesh> m_mesh;
    std::shared_ptr<scene::Mesh> m_waterMesh;
    std::shared_ptr<scene::Mesh> m_crtMesh;
    std::shared_ptr<gl::Texture2D<gl::SRGBA8>> m_colorBuffer;
    std::shared_ptr<gl::Texture2D<gl::ScalarByte>> m_noise;
    std::shared_ptr<gl::Framebuffer> m_fb;
  };

  RenderSettings m_renderSettings{};
  glm::ivec2 m_size{-1};
  std::shared_ptr<PortalStage> m_portalStage;
  std::shared_ptr<GeometryStage> m_geometryStage;
  std::shared_ptr<SSAOStage> m_ssaoStage;
  std::shared_ptr<FXAAStage> m_fxaaStage;
  std::shared_ptr<CompositionStage> m_compositionStage;

public:
  void bindGeometryFrameBuffer(const glm::ivec2& size)
  {
    BOOST_ASSERT(m_geometryStage != nullptr);
    m_geometryStage->bind(size);
  }

  void bindPortalFrameBuffer()
  {
    BOOST_ASSERT(m_portalStage != nullptr);
    BOOST_ASSERT(m_geometryStage != nullptr);
    m_portalStage->bind(*m_geometryStage->getDepthBuffer());
  }

  explicit RenderPipeline(scene::MaterialManager& materialManager, const glm::ivec2& viewport);

  void compositionPass(bool water);

  void updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera);

  void resize(scene::MaterialManager& materialManager, const glm::ivec2& viewport, bool force = false);

  void apply(const RenderSettings& renderSettings, scene::MaterialManager& materialManager);
};
} // namespace render
