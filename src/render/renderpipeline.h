#pragma once

#include "gl/debuggroup.h"
#include "gl/framebuffer.h"
#include "gl/pixel.h"
#include "gl/texture2d.h"
#include "gl/texturedepth.h"
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
  const std::shared_ptr<scene::ShaderProgram> m_fxaaShader;
  const std::shared_ptr<scene::Material> m_fxaaMaterial;
  const std::shared_ptr<scene::ShaderProgram> m_ssaoShader;
  const std::shared_ptr<scene::Material> m_ssaoMaterial;
  const std::shared_ptr<scene::ShaderProgram> m_ssaoBlurShader;
  const std::shared_ptr<scene::Material> m_ssaoBlurMaterial;
  const std::shared_ptr<scene::ShaderProgram> m_fxDarknessShader;
  const std::shared_ptr<scene::Material> m_fxDarknessMaterial;
  const std::shared_ptr<scene::ShaderProgram> m_fxWaterDarknessShader;
  const std::shared_ptr<scene::Material> m_fxWaterDarknessMaterial;

  const std::shared_ptr<scene::Model> m_fbModel = std::make_shared<scene::Model>();

  const std::shared_ptr<gl::TextureDepth<float>> m_portalDepthBuffer
    = std::make_shared<gl::TextureDepth<float>>("portal-depth");
  const std::shared_ptr<gl::Texture2D<gl::RG32F>> m_portalPerturbBuffer
    = std::make_shared<gl::Texture2D<gl::RG32F>>("portal-perturb");
  std::shared_ptr<gl::Framebuffer> m_portalFb;

  const std::shared_ptr<gl::TextureDepth<float>> m_geometryDepthBuffer
    = std::make_shared<gl::TextureDepth<float>>("geometry-depth");
  const std::shared_ptr<gl::Texture2D<gl::SRGBA8>> m_geometryColorBuffer
    = std::make_shared<gl::Texture2D<gl::SRGBA8>>("geometry-color");
  const std::shared_ptr<gl::Texture2D<gl::RGB32F>> m_geometryPositionBuffer
    = std::make_shared<gl::Texture2D<gl::RGB32F>>("geometry-position");
  const std::shared_ptr<gl::Texture2D<gl::RGB16F>> m_geometryNormalBuffer
    = std::make_shared<gl::Texture2D<gl::RGB16F>>("geometry-normal");
  std::shared_ptr<gl::Framebuffer> m_geometryFb;

  const std::shared_ptr<gl::Texture2D<gl::RGB32F>> m_ssaoNoiseTexture
    = std::make_shared<gl::Texture2D<gl::RGB32F>>("ssao-noise");
  const std::shared_ptr<gl::Texture2D<gl::Scalar32F>> m_ssaoAOBuffer
    = std::make_shared<gl::Texture2D<gl::Scalar32F>>("ssao-ao");
  std::shared_ptr<gl::Framebuffer> m_ssaoFb;

  const std::shared_ptr<gl::Texture2D<gl::Scalar32F>> m_ssaoBlurAOBuffer
    = std::make_shared<gl::Texture2D<gl::Scalar32F>>("ssao-blur-ao");
  std::shared_ptr<gl::Framebuffer> m_ssaoBlurFb;

  const std::shared_ptr<gl::Texture2D<gl::SRGBA8>> m_fxaaColorBuffer
    = std::make_shared<gl::Texture2D<gl::SRGBA8>>("fxaa-color");
  std::shared_ptr<gl::Framebuffer> m_fxaaFb;

public:
  // ReSharper disable once CppMemberFunctionMayBeConst
  void bindGeometryFrameBuffer(const glm::ivec2& size)
  {
    gl::Framebuffer::unbindAll();
    GL_ASSERT(::gl::viewport(0, 0, size.x, size.y));
    m_geometryFb->bindWithAttachments();
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void bindPortalFrameBuffer()
  {
    gl::Framebuffer::unbindAll();
    m_portalDepthBuffer->copyFrom(*m_geometryDepthBuffer);
    m_portalFb->bindWithAttachments();
  }

  explicit RenderPipeline(scene::ShaderManager& shaderManager, const glm::ivec2& viewport);

  // ReSharper disable once CppMemberFunctionMayBeConst
  void finalPass(bool water);

  // ReSharper disable once CppMemberFunctionMayBeConst
  void update(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera,
              const std::chrono::high_resolution_clock::time_point& time);

  // ReSharper disable once CppMemberFunctionMayBeConst
  void resize(const glm::ivec2& viewport);
};
} // namespace render
