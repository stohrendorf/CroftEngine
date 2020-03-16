#pragma once

#include "gl/debuggroup.h"
#include "gl/framebuffer.h"
#include "gl/pixel.h"
#include "gl/texture2d.h"
#include "gl/texturedepth.h"
#include "scene/blur.h"
#include "scene/camera.h"
#include "scene/material.h"
#include "scene/model.h"
#include "scene/rendercontext.h"

#include <chrono>
#include <memory>

namespace render
{
namespace scene
{
class ShaderManager;
}
class RenderPipeline
{
public:
  static constexpr bool FlushStages = false;

private:
  static std::shared_ptr<scene::Model> makeFbModel();

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
    const std::shared_ptr<scene::Model> renderModel = makeFbModel();

    const std::shared_ptr<scene::ShaderProgram> shader;
    const std::shared_ptr<scene::Material> material;

    std::shared_ptr<gl::Texture2D<gl::RGB32F>> noiseTexture;
    std::shared_ptr<gl::Texture2D<gl::ScalarByte>> aoBuffer;
    std::shared_ptr<gl::Framebuffer> fb;

    scene::SeparableBlur<gl::ScalarByte, 4> blur;

    explicit SSAOStage(scene::ShaderManager& shaderManager);
    void resize(const glm::ivec2& viewport, const GeometryStage& geometryStage);
    void update(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera);

    void render(const glm::ivec2& size);
  };

  struct FXAAStage
  {
    const std::shared_ptr<scene::Model> model = makeFbModel();

    const std::shared_ptr<scene::ShaderProgram> shader;
    const std::shared_ptr<scene::Material> material;
    std::shared_ptr<gl::Texture2D<gl::SRGBA8>> colorBuffer;
    std::shared_ptr<gl::Framebuffer> fb;

    explicit FXAAStage(scene::ShaderManager& shaderManager);

    void bind();
    void resize(const glm::ivec2& viewport, const GeometryStage& geometryStage);

    void render(const glm::ivec2& size);
  };

  struct PostprocessStage
  {
    const std::shared_ptr<scene::Model> model = makeFbModel();
    const std::shared_ptr<scene::Model> waterModel = makeFbModel();

    const std::shared_ptr<scene::ShaderProgram> darknessShader;
    const std::shared_ptr<scene::Material> darknessMaterial;
    const std::shared_ptr<scene::ShaderProgram> waterDarknessShader;
    const std::shared_ptr<scene::Material> waterDarknessMaterial;

    explicit PostprocessStage(scene::ShaderManager& shaderManager);

    void update(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera,
                const std::chrono::high_resolution_clock::time_point& time);

    void resize(const glm::ivec2& viewport,
                const GeometryStage& geometryStage,
                const PortalStage& portalStage,
                const SSAOStage& ssaoStage,
                const FXAAStage& fxaaStage);

    void render(bool water);
  };

  PortalStage m_portalStage;
  GeometryStage m_geometryStage;
  SSAOStage m_ssaoStage;
  FXAAStage m_fxaaStage;
  PostprocessStage m_postprocessStage;

public:
  // ReSharper disable once CppMemberFunctionMayBeConst
  void bindGeometryFrameBuffer(const glm::ivec2& size)
  {
    m_geometryStage.bind(size);
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void bindPortalFrameBuffer()
  {
    m_portalStage.bind(*m_geometryStage.depthBuffer);
  }

  explicit RenderPipeline(scene::ShaderManager& shaderManager, const glm::ivec2& viewport);

  // ReSharper disable once CppMemberFunctionMayBeConst
  void finalPass(bool water);

  // ReSharper disable once CppMemberFunctionMayBeConst
  void update(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera,
              const std::chrono::high_resolution_clock::time_point& time);

  void resize(const glm::ivec2& viewport)
  {
    if(m_size == viewport)
    {
      return;
    }

    m_size = viewport;

    resizeTextures(viewport);
  }

private:
  glm::ivec2 m_size{-1};

  void resizeTextures(const glm::ivec2& viewport);
};
} // namespace render
