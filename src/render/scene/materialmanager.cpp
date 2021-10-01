#include "materialmanager.h"

#include "bufferparameter.h"
#include "csm.h"
#include "material.h"
#include "node.h"
#include "renderer.h"
#include "shadercache.h"
#include "uniformparameter.h"

#include <algorithm>
#include <boost/assert.hpp>
#include <chrono>
#include <cstdint>
#include <gl/glad_init.h>
#include <gl/pixel.h>
#include <gl/program.h>
#include <gl/renderstate.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texture2darray.h> // IWYU pragma: keep
#include <gl/texturehandle.h>
#include <glm/vec2.hpp>
#include <gslu.h>
#include <random>
#include <utility>
#include <vector>

namespace render::scene
{
class Mesh;

namespace
{
void configureForScreenSpaceEffect(Material& m, bool enableBlend)
{
  m.getRenderState().setDepthTest(false);
  m.getRenderState().setDepthWrite(false);
  m.getRenderState().setBlend(0, enableBlend);
  if(enableBlend)
    m.getRenderState().setBlendFactors(0, gl::api::BlendingFactor::SrcAlpha, gl::api::BlendingFactor::OneMinusSrcAlpha);
}
} // namespace

gsl::not_null<std::shared_ptr<Material>> MaterialManager::getSprite(bool billboard)
{
  if(auto it = m_sprite.find(billboard); it != m_sprite.end())
    return it->second;

  auto m = gslu::make_nn_shared<Material>(m_shaderCache->getGeometry(false, false, true, billboard ? 2 : 1));
  m->getRenderState().setCullFace(false);

  m->getUniformBlock("Transform")->bindTransformBuffer();
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  m->getUniform("u_diffuseTextures")
    ->bind([this](const Node& /*node*/, const Mesh& /*mesh*/, gl::Uniform& uniform)
           { uniform.set(gsl::not_null{m_geometryTextures}); });

  m_sprite.emplace(billboard, m);
  return m;
}

gsl::not_null<std::shared_ptr<Material>> MaterialManager::getCSMDepthOnly(bool skeletal)
{
  if(auto it = m_csmDepthOnly.find(skeletal); it != m_csmDepthOnly.end())
    return it->second;

  auto m = gslu::make_nn_shared<Material>(m_shaderCache->getCSMDepthOnly(skeletal));
  m->getUniform("u_mvp")->bind(
    [this](const Node& node, const Mesh& /*mesh*/, gl::Uniform& uniform)
    {
      BOOST_ASSERT(m_csm != nullptr);
      uniform.set(m_csm->getActiveMatrix(node.getModelMatrix()));
    });
  m->getRenderState().setDepthTest(true);
  m->getRenderState().setDepthWrite(true);
  if(auto buffer = m->tryGetBuffer("BoneTransform"))
    buffer->bindBoneTransformBuffer();

  m_csmDepthOnly.emplace(skeletal, m);
  return m;
}

gsl::not_null<std::shared_ptr<Material>> MaterialManager::getDepthOnly(bool skeletal)
{
  if(auto it = m_depthOnly.find(skeletal); it != m_depthOnly.end())
    return it->second;

  auto m = gslu::make_nn_shared<Material>(m_shaderCache->getDepthOnly(skeletal));
  m->getRenderState().setDepthTest(true);
  m->getRenderState().setDepthWrite(true);
  m->getUniformBlock("Transform")->bindTransformBuffer();
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  if(auto buffer = m->tryGetBuffer("BoneTransform"))
    buffer->bindBoneTransformBuffer();
  m->getUniform("u_diffuseTextures")
    ->bind([this](const Node& /*node*/, const Mesh& /*mesh*/, gl::Uniform& uniform)
           { uniform.set(gsl::not_null{m_geometryTextures}); });

  m_depthOnly.emplace(skeletal, m);
  return m;
}

gsl::not_null<std::shared_ptr<Material>> MaterialManager::getGeometry(bool inWater, bool skeletal, bool roomShadowing)
{
  Expects(m_geometryTextures != nullptr);
  const std::tuple key{inWater, skeletal, roomShadowing};
  if(auto it = m_geometry.find(key); it != m_geometry.end())
    return it->second;

  auto m = gslu::make_nn_shared<Material>(m_shaderCache->getGeometry(inWater, skeletal, roomShadowing, 0));
  m->getUniform("u_diffuseTextures")
    ->bind([this](const Node& /*node*/, const Mesh& /*mesh*/, gl::Uniform& uniform)
           { uniform.set(gsl::not_null{m_geometryTextures}); });

  m->getUniformBlock("Transform")->bindTransformBuffer();
  if(auto buffer = m->tryGetBuffer("BoneTransform"))
    buffer->bindBoneTransformBuffer();
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  m->getUniformBlock("CSM")->bind(
    [this](const Node& node, const Mesh& /*mesh*/, gl::UniformBlock& ub)
    {
      BOOST_ASSERT(m_csm != nullptr);
      ub.bind(m_csm->getBuffer(node.getModelMatrix()));
    });

  m->getUniform("u_csmVsm[0]")
    ->bind(
      [this](const Node& /*node*/, const Mesh& /*mesh*/, gl::Uniform& uniform)
      {
        BOOST_ASSERT(m_csm != nullptr);
        uniform.set(gsl::make_span(m_csm->getTextures()));
      });

  if(auto uniform = m->tryGetUniform("u_noise"))
    uniform->set(gsl::not_null{m_noiseTexture});

  if(auto uniform = m->tryGetUniform("u_time"))
  {
    uniform->bind(
      [renderer = m_renderer](const Node&, const Mesh& /*mesh*/, gl::Uniform& uniform)
      {
        const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
        uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
      });
  }

  m_geometry.emplace(key, m);
  return m;
}

gsl::not_null<std::shared_ptr<Material>> MaterialManager::getWaterSurface()
{
  if(m_waterSurface != nullptr)
    return gsl::not_null{m_waterSurface};

  m_waterSurface = std::make_shared<Material>(m_shaderCache->getWaterSurface());
  m_waterSurface->getRenderState().setCullFace(false);
  m_waterSurface->getRenderState().setBlend(0, false);
  m_waterSurface->getRenderState().setBlend(1, false);

  m_waterSurface->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  m_waterSurface->getUniform("u_time")->bind(
    [renderer = m_renderer](const Node&, const Mesh& /*mesh*/, gl::Uniform& uniform)
    {
      const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
      uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
    });

  m_waterSurface->getUniform("u_noise")->set(gsl::not_null{m_noiseTexture});

  return gsl::not_null{m_waterSurface};
}

gsl::not_null<std::shared_ptr<Material>> MaterialManager::getLightning()
{
  if(m_lightning != nullptr)
    return gsl::not_null{m_lightning};

  m_lightning = std::make_shared<render::scene::Material>(m_shaderCache->getLightning());
  m_lightning->getUniformBlock("Transform")->bindTransformBuffer();
  m_lightning->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());

  return gsl::not_null{m_lightning};
}

MaterialManager::MaterialManager(gsl::not_null<std::shared_ptr<ShaderCache>> shaderCache,
                                 gsl::not_null<std::shared_ptr<Renderer>> renderer)
    : m_shaderCache{std::move(shaderCache)}
    , m_renderer{std::move(renderer)}
{
  static constexpr int NoiseTextureSize = 128;
  std::vector<gl::RGB8> noiseData;
  noiseData.resize(NoiseTextureSize * NoiseTextureSize);
  std::default_random_engine generator{}; // NOLINT(cert-msc32-c, cert-msc51-cpp)
  std::uniform_int_distribution<uint16_t> randomInts(0, 255);
  for(auto& i : noiseData)
  {
    const auto value = randomInts(generator);
    i = gl::RGB8{gsl::narrow_cast<uint8_t>(value), gsl::narrow_cast<uint8_t>(value), gsl::narrow_cast<uint8_t>(value)};
  }

  auto noiseTexture = gslu::make_nn_shared<gl::Texture2D<gl::RGB8>>(glm::ivec2{NoiseTextureSize}, "noise");
  noiseTexture->assign(noiseData);
  m_noiseTexture = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::RGB8>>>(
    noiseTexture,
    gslu::make_nn_unique<gl::Sampler>("noise")
      | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::MirroredRepeat)
      | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::MirroredRepeat)
      | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear));
}

gsl::not_null<std::shared_ptr<Material>> MaterialManager::getWorldComposition(bool inWater, bool dof, bool hbao)
{
  const std::tuple key{inWater, dof, hbao};
  if(auto it = m_composition.find(key); it != m_composition.end())
    return it->second;
  auto m = gslu::make_nn_shared<Material>(m_shaderCache->getWorldComposition(inWater, dof, hbao));

  if(auto uniform = m->tryGetUniform("u_time"))
  {
    uniform->bind(
      [renderer = m_renderer](const Node&, const Mesh& /*mesh*/, gl::Uniform& uniform)
      {
        const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
        uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
      });
  }

  if(auto uniform = m->tryGetUniform("u_noise"))
    uniform->set(gsl::not_null{m_noiseTexture});

  m_composition.emplace(key, m);
  return m;
}

gsl::not_null<std::shared_ptr<Material>> MaterialManager::getUi()
{
  if(m_ui != nullptr)
    return gsl::not_null{m_ui};

  auto m = std::make_shared<Material>(m_shaderCache->getUi());
  m->getUniform("u_input")->bind([this](const Node& /*node*/, const Mesh& /*mesh*/, gl::Uniform& uniform)
                                 { uniform.set(gsl::not_null{m_geometryTextures}); });
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  configureForScreenSpaceEffect(*m, true);
  m_ui = m;
  return gsl::not_null{m_ui};
}

gsl::not_null<std::shared_ptr<Material>> MaterialManager::getFlat(bool withAlpha, bool invertY, bool withAspectRatio)
{
  const std::tuple key{withAlpha, invertY, withAspectRatio};
  if(auto it = m_flat.find(key); it != m_flat.end())
    return it->second;

  auto m = gslu::make_nn_shared<Material>(m_shaderCache->getFlat(withAlpha, invertY, withAspectRatio));
  m->getRenderState().setBlend(0, withAlpha);
  m->getRenderState().setBlendFactors(0, gl::api::BlendingFactor::SrcAlpha, gl::api::BlendingFactor::OneMinusSrcAlpha);
  if(const auto uniformBlock = m->tryGetUniformBlock("Camera"))
    uniformBlock->bindCameraBuffer(m_renderer->getCamera());
  configureForScreenSpaceEffect(*m, withAlpha);
  m_flat.emplace(key, m);
  return m;
}

const std::shared_ptr<Material>& MaterialManager::getBackdrop()
{
  if(m_backdrop != nullptr)
    return m_backdrop;

  auto m = std::make_shared<Material>(m_shaderCache->getBackdrop());
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  configureForScreenSpaceEffect(*m, false);
  m_backdrop = m;
  return m_backdrop;
}

gsl::not_null<std::shared_ptr<Material>> MaterialManager::getFXAA()
{
  if(m_fxaa != nullptr)
    return gsl::not_null{m_fxaa};

  auto m = gslu::make_nn_shared<Material>(m_shaderCache->getFXAA());
  configureForScreenSpaceEffect(*m, false);
  configureForScreenSpaceEffect(*m, false);
  m_fxaa = m;
  return m;
}

gsl::not_null<std::shared_ptr<Material>> MaterialManager::getCRT()
{
  if(m_crt != nullptr)
    return gsl::not_null{m_crt};

  auto m = gslu::make_nn_shared<Material>(m_shaderCache->getCRT());
  m->getUniform("u_time")->bind(
    [renderer = m_renderer](const Node&, const Mesh& /*mesh*/, gl::Uniform& uniform)
    {
      const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
      uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
    });
  m->getUniform("u_noise")->set(gsl::not_null{m_noiseTexture});
  configureForScreenSpaceEffect(*m, false);
  m_crt = m;
  return m;
}

gsl::not_null<std::shared_ptr<Material>> MaterialManager::getVelvia()
{
  if(m_velvia != nullptr)
    return gsl::not_null{m_velvia};

  auto m = gslu::make_nn_shared<Material>(m_shaderCache->getVelvia());
  configureForScreenSpaceEffect(*m, false);
  configureForScreenSpaceEffect(*m, false);
  m_velvia = m;
  return m;
}

gsl::not_null<std::shared_ptr<Material>> MaterialManager::getFilmGrain()
{
  if(m_filmGrain != nullptr)
    return gsl::not_null{m_filmGrain};

  auto m = gslu::make_nn_shared<Material>(m_shaderCache->getFilmGrain());
  m->getUniform("u_time")->bind(
    [renderer = m_renderer](const Node&, const Mesh& /*mesh*/, gl::Uniform& uniform)
    {
      const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
      uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
    });
  m->getUniform("u_noise")->set(gsl::not_null{m_noiseTexture});
  configureForScreenSpaceEffect(*m, false);
  m_filmGrain = m;
  return m;
}

gsl::not_null<std::shared_ptr<Material>> MaterialManager::getLensDistortion()
{
  if(m_lensDistortion != nullptr)
    return gsl::not_null{m_lensDistortion};

  auto m = gslu::make_nn_shared<Material>(m_shaderCache->getLensDistortion());
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());

  m_lensDistortion = m;
  return m;
}

gsl::not_null<std::shared_ptr<Material>> MaterialManager::getHBAO()
{
  if(m_hbao != nullptr)
    return gsl::not_null{m_hbao};

  auto m = gslu::make_nn_shared<Material>(m_shaderCache->getHBAO());
  m->getUniform("u_noise")->set(gsl::not_null{m_noiseTexture});
  configureForScreenSpaceEffect(*m, false);
  m_hbao = m;
  return m;
}

gsl::not_null<std::shared_ptr<Material>> MaterialManager::getVSMSquare()
{
  if(m_vsmSquare != nullptr)
    return gsl::not_null{m_vsmSquare};

  auto m = std::make_shared<Material>(m_shaderCache->getVSMSquare());
  configureForScreenSpaceEffect(*m, false);
  m_vsmSquare = m;
  return gsl::not_null{m_vsmSquare};
}

void MaterialManager::setGeometryTextures(
  std::shared_ptr<gl::TextureHandle<gl::Texture2DArray<gl::SRGBA8>>> geometryTextures)
{
  m_geometryTextures = std::move(geometryTextures);
}

void MaterialManager::setFiltering(bool bilinear, float anisotropyLevel)
{
  if(m_geometryTextures == nullptr)
    return;

  auto sampler = gslu::make_nn_unique<gl::Sampler>("geometry-sampler");
  if(bilinear)
  {
    sampler->set(gl::api::TextureMinFilter::LinearMipmapLinear).set(gl::api::TextureMagFilter::Linear);
  }
  else
  {
    sampler->set(gl::api::TextureMinFilter::NearestMipmapLinear).set(gl::api::TextureMagFilter::Nearest);
  }

  if(anisotropyLevel != 0 && gl::hasAnisotropicFilteringExtension())
    sampler->set(gl::api::SamplerParameterF::TextureMaxAnisotropy, anisotropyLevel);

  m_geometryTextures = std::make_shared<gl::TextureHandle<gl::Texture2DArray<gl::SRGBA8>>>(
    m_geometryTextures->getTexture(), std::move(sampler));
}

gsl::not_null<std::shared_ptr<Material>>
  MaterialManager::getFastGaussBlur(uint8_t extent, uint8_t blurDir, uint8_t blurDim)
{
  const std::tuple key{extent, blurDir, blurDim};
  if(auto it = m_fastGaussBlur.find(key); it != m_fastGaussBlur.end())
    return it->second;

  auto m = gslu::make_nn_shared<Material>(m_shaderCache->getFastGaussBlur(extent, blurDim));
  configureForScreenSpaceEffect(*m, false);
  m->getUniform("u_blurDir")->set(int(blurDir));
  m_fastGaussBlur.emplace(key, m);
  return m;
}

gsl::not_null<std::shared_ptr<Material>>
  MaterialManager::getFastBoxBlur(uint8_t extent, uint8_t blurDir, uint8_t blurDim)
{
  const std::tuple key{extent, blurDir, blurDim};
  if(auto it = m_fastBoxBlur.find(key); it != m_fastBoxBlur.end())
    return it->second;

  auto m = gslu::make_nn_shared<Material>(m_shaderCache->getFastBoxBlur(extent, blurDim));
  configureForScreenSpaceEffect(*m, false);
  m->getUniform("u_blurDir")->set(int(blurDir));
  m_fastBoxBlur.emplace(key, m);
  return m;
}
} // namespace render::scene
