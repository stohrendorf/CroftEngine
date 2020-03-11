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
public:
  static constexpr bool FlushStages = false;

private:
  static std::shared_ptr<scene::Model> makeFbModel();

  template<typename T>
  struct SingleBlur
  {
    using Texture = gl::Texture2D<gl::Scalar<T>>;

    const std::string name;
    std::shared_ptr<Texture> buffer;
    const std::shared_ptr<scene::Model> model = makeFbModel();
    const std::shared_ptr<scene::ShaderProgram> shader;
    const std::shared_ptr<scene::Material> material;
    std::shared_ptr<gl::Framebuffer> fb;

    explicit SingleBlur(std::string name, scene::ShaderManager& shaderManager, uint8_t n)
        : name{std::move(name)}
        , shader{shaderManager.getBlur(2, n)}
        , material{std::make_shared<scene::Material>(shader)}
    {
    }

    void build()
    {
      fb = gl::FrameBufferBuilder()
             .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, buffer)
             .build(name + "/fb");
    }

    void resize(const glm::ivec2& viewport, const std::shared_ptr<Texture>& src)
    {
      buffer = std::make_shared<Texture>(viewport, name + "/blur");
      buffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
        .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
        .set(gl::api::TextureMinFilter::Linear)
        .set(gl::api::TextureMagFilter::Linear);

      material->getUniform("u_input")->set(src);

      model->getMeshes().clear();
      model->addMesh(scene::createQuadFullscreen(
        gsl::narrow<float>(viewport.x), gsl::narrow<float>(viewport.y), shader->getHandle()));
      model->getMeshes()[0]->getMaterial().set(scene::RenderMode::Full, material);
    }

    void render(const glm::ivec2& size) const
    {
      gl::DebugGroup dbg{name + "/blur-pass"};
      GL_ASSERT(gl::api::viewport(0, 0, size.x, size.y));

      gl::RenderState state;
      state.setBlend(false);
      state.apply(true);
      scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
      scene::Node dummyNode{""};
      context.setCurrentNode(&dummyNode);

      fb->bindWithAttachments();
      model->render(context);
    }
  };

  template<typename T>
  struct BidirBlur
  {
    using Texture = gl::Texture2D<gl::Scalar<T>>;

    SingleBlur<T> blur1;
    SingleBlur<T> blur2;

    explicit BidirBlur(const std::string& name, scene::ShaderManager& shaderManager)
        : blur1{name + "/blur-1", shaderManager, 1}
        , blur2{name + "/blur-2", shaderManager, 2}
    {
    }

    void build()
    {
      blur1.build();
      blur2.build();
    }

    void resize(const glm::ivec2& viewport, const std::shared_ptr<Texture>& src)
    {
      blur1.resize(viewport, src);
      blur2.resize(viewport, blur1.buffer);
    }

    void render(const glm::ivec2& size) const
    {
      blur1.render(size);
      blur2.render(size);
      blur1.fb->invalidate();
    }
  };

  struct PortalStage
  {
    std::shared_ptr<gl::TextureDepth<float>> depthBuffer;
    std::shared_ptr<gl::Texture2D<gl::RG32F>> perturbBuffer;
    std::shared_ptr<gl::Framebuffer> fb;

    void resize(const glm::ivec2& viewport);
    void bind(const gl::TextureDepth<float>& depth);
    void build();
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
    void build();
  };

  struct SSAOStage
  {
    const std::shared_ptr<scene::Model> renderModel = makeFbModel();

    const std::shared_ptr<scene::ShaderProgram> shader;
    const std::shared_ptr<scene::Material> material;

    std::shared_ptr<gl::Texture2D<gl::RGB32F>> noiseTexture;
    std::shared_ptr<gl::Texture2D<gl::ScalarByte>> aoBuffer;
    std::shared_ptr<gl::Framebuffer> fb;

    BidirBlur<uint8_t> blur;

    explicit SSAOStage(scene::ShaderManager& shaderManager);
    void build();
    void resize(const glm::ivec2& viewport, const GeometryStage& geometryStage);
    void update(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera);

    void render(const glm::ivec2& size) const;
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
    void build();
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
    buildFramebuffers();
  }

private:
  glm::ivec2 m_size{-1};

  void resizeTextures(const glm::ivec2& viewport);
  void buildFramebuffers();
};
} // namespace render
