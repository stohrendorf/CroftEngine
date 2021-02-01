#pragma once

#include "gl/debuggroup.h"
#include "gl/framebuffer.h"
#include "gl/pixel.h"
#include "gl/texture2d.h"
#include "gl/texturedepth.h"
#include "scene/blur.h"
#include "scene/camera.h"
#include "scene/material.h"
#include "scene/rendercontext.h"

#include <chrono>
#include <memory>

namespace render
{
namespace scene
{
class ShaderManager;
class MaterialManager;
} // namespace scene

struct RenderSettings
{
  bool crt = true;
  bool dof = true;
  bool lensDistortion = true;
};

class RenderPipeline
{
public:
  static constexpr bool FlushStages = false;

private:
  static std::shared_ptr<scene::Mesh> createFbMesh(const glm::ivec2& size, const gl::Program& program);

  struct PortalStage
  {
    std::shared_ptr<gl::TextureDepth<float>> depthBuffer;
    std::shared_ptr<gl::Texture2D<gl::RG32F>> perturbBuffer;
    std::shared_ptr<gl::Framebuffer> fb;

    void resize(const glm::ivec2& viewport);
    void bind(const gl::TextureDepth<float>& depth);
  };

  struct GeometryStage
  {
    std::shared_ptr<gl::TextureDepth<float>> depthBuffer;
    std::shared_ptr<gl::Texture2D<gl::SRGBA8>> colorBuffer;
    std::shared_ptr<gl::Texture2D<gl::RGB32F>> positionBuffer;
    std::shared_ptr<gl::Texture2D<gl::RGB16F>> normalBuffer;
    std::shared_ptr<gl::Framebuffer> fb;

    void resize(const glm::ivec2& viewport);
    void bind(const glm::ivec2& size);
  };

  struct SSAOStage
  {
    std::shared_ptr<scene::Mesh> renderMesh;

    const std::shared_ptr<scene::ShaderProgram> shader;
    const std::shared_ptr<scene::Material> material;

    std::shared_ptr<gl::Texture2D<gl::RGB32F>> noiseTexture;
    std::shared_ptr<gl::Texture2D<gl::Scalar16F>> aoBuffer;
    std::shared_ptr<gl::Framebuffer> fb;

    scene::SeparableBlur<gl::Scalar16F> blur;

    explicit SSAOStage(scene::ShaderManager& shaderManager);
    void resize(const glm::ivec2& viewport, const GeometryStage& geometryStage);
    void updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera);

    void render(const glm::ivec2& size);
  };

  struct FXAAStage
  {
    std::shared_ptr<scene::Mesh> mesh;

    const std::shared_ptr<scene::ShaderProgram> shader;
    const std::shared_ptr<scene::Material> material;
    std::shared_ptr<gl::Texture2D<gl::SRGBA8>> colorBuffer;
    std::shared_ptr<gl::Framebuffer> fb;

    explicit FXAAStage(scene::ShaderManager& shaderManager);

    void bind();
    void resize(const glm::ivec2& viewport, const GeometryStage& geometryStage);

    void render(const glm::ivec2& size);
  };

  struct CompositionStage
  {
    std::shared_ptr<scene::Mesh> mesh;
    std::shared_ptr<scene::Mesh> waterMesh;
    std::shared_ptr<scene::Mesh> crtMesh;

    std::shared_ptr<scene::Material> compositionMaterial;
    std::shared_ptr<scene::Material> waterCompositionMaterial;
    const std::shared_ptr<scene::Material> crtMaterial;
    std::shared_ptr<gl::Texture2D<gl::SRGBA8>> colorBuffer;
    std::shared_ptr<gl::Texture2D<gl::ScalarByte>> noise;
    std::shared_ptr<gl::Framebuffer> fb;

    explicit CompositionStage(scene::MaterialManager& materialManager, const RenderSettings& renderSettings);

    void updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera);

    void resize(const glm::ivec2& viewport,
                const GeometryStage& geometryStage,
                const PortalStage& portalStage,
                const SSAOStage& ssaoStage,
                const FXAAStage& fxaaStage);

    void render(bool water, const RenderSettings& renderSettings);

    void initMaterials(scene::MaterialManager& materialManager, const RenderSettings& renderSettings);
  };

  RenderSettings m_renderSettings{};
  glm::ivec2 m_size{-1};
  PortalStage m_portalStage;
  GeometryStage m_geometryStage;
  SSAOStage m_ssaoStage;
  FXAAStage m_fxaaStage;
  CompositionStage m_compositionStage;

  void resizeTextures(const glm::ivec2& viewport);

public:
  void bindGeometryFrameBuffer(const glm::ivec2& size)
  {
    m_geometryStage.bind(size);
  }

  void bindPortalFrameBuffer()
  {
    m_portalStage.bind(*m_geometryStage.depthBuffer);
  }

  explicit RenderPipeline(scene::MaterialManager& materialManager, const glm::ivec2& viewport);

  void compositionPass(bool water);

  void updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera);

  void resize(const glm::ivec2& viewport, bool force = false)
  {
    if(!force && m_size == viewport)
    {
      return;
    }

    m_size = viewport;

    resizeTextures(viewport);
  }

  [[nodiscard]] const auto& getRenderSettings() const noexcept
  {
    return m_renderSettings;
  }

  void toggleCrt()
  {
    m_renderSettings.crt = !m_renderSettings.crt;
  }

  void toggleDof(scene::MaterialManager& materialManager)
  {
    m_renderSettings.dof = !m_renderSettings.dof;
    m_compositionStage.initMaterials(materialManager, m_renderSettings);
    resize(m_size, true);
  }

  void toggleLensDistortion(scene::MaterialManager& materialManager)
  {
    m_renderSettings.lensDistortion = !m_renderSettings.lensDistortion;
    m_compositionStage.initMaterials(materialManager, m_renderSettings);
    resize(m_size, true);
  }
};
} // namespace render
