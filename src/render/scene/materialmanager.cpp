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
#include <gl/glew_init.h>
#include <gl/pixel.h>
#include <gl/program.h>
#include <gl/renderstate.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texture2darray.h> // IWYU pragma: keep
#include <gl/texturehandle.h>
#include <glm/vec2.hpp>
#include <random>
#include <utility>
#include <vector>

namespace render::scene
{
class Mesh;

namespace
{
void configureForScreenSpaceEffect(Material& m, bool enableBlend = false)
{
  m.getRenderState().setDepthTest(false);
  m.getRenderState().setDepthWrite(false);
  if(enableBlend)
  {
    m.getRenderState().setBlend(true);
    m.getRenderState().setBlendFactors(gl::api::BlendingFactor::SrcAlpha, gl::api::BlendingFactor::OneMinusSrcAlpha);
  }
}
} // namespace

std::shared_ptr<Material> MaterialManager::getSprite(bool billboard)
{
  if(const auto& tmp = m_sprite[billboard])
    return tmp;

  auto m = std::make_shared<Material>(m_shaderCache->getGeometry(false, false, true));
  m->getRenderState().setCullFace(false);

  m->getUniformBlock("Transform")->bindTransformBuffer();
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  m->getUniform("u_isSprite")->set(billboard ? 2 : 1);

  m_sprite[billboard] = m;
  return m;
}

const std::shared_ptr<Material>& MaterialManager::getCSMDepthOnly(bool skeletal)
{
  if(const auto& tmp = m_csmDepthOnly[skeletal])
    return tmp;

  m_csmDepthOnly[skeletal] = std::make_shared<Material>(m_shaderCache->getCSMDepthOnly(skeletal));
  m_csmDepthOnly[skeletal]->getUniform("u_mvp")->bind(
    [this](const Node& node, const Mesh& /*mesh*/, gl::Uniform& uniform)
    {
      BOOST_ASSERT(m_csm != nullptr);
      uniform.set(m_csm->getActiveMatrix(node.getModelMatrix()));
    });
  m_csmDepthOnly[skeletal]->getRenderState().setDepthTest(true);
  m_csmDepthOnly[skeletal]->getRenderState().setDepthWrite(true);
  if(auto buffer = m_csmDepthOnly[skeletal]->tryGetBuffer("BoneTransform"))
    buffer->bindBoneTransformBuffer();

  return m_csmDepthOnly[skeletal];
}

const std::shared_ptr<Material>& MaterialManager::getDepthOnly(bool skeletal)
{
  if(const auto& tmp = m_depthOnly[skeletal])
    return tmp;

  m_depthOnly[skeletal] = std::make_shared<Material>(m_shaderCache->getDepthOnly(skeletal));
  m_depthOnly[skeletal]->getRenderState().setDepthTest(true);
  m_depthOnly[skeletal]->getRenderState().setDepthWrite(true);
  m_depthOnly[skeletal]->getUniformBlock("Transform")->bindTransformBuffer();
  m_depthOnly[skeletal]->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  if(auto buffer = m_depthOnly[skeletal]->tryGetBuffer("BoneTransform"))
    buffer->bindBoneTransformBuffer();
  m_depthOnly[skeletal]
    ->getUniform("u_diffuseTextures")
    ->bind([this](const Node& /*node*/, const Mesh& /*mesh*/, gl::Uniform& uniform)
           { uniform.set(m_geometryTextures); });

  return m_depthOnly[skeletal];
}

std::shared_ptr<Material> MaterialManager::getGeometry(bool water, bool skeletal, bool roomShadowing)
{
  Expects(m_geometryTextures != nullptr);
  const std::tuple key{water, skeletal, roomShadowing};
  if(auto it = m_geometry.find(key); it != m_geometry.end())
    return it->second;

  auto m = std::make_shared<Material>(m_shaderCache->getGeometry(water, skeletal, roomShadowing));
  m->getUniform("u_diffuseTextures")
    ->bind([this](const Node& /*node*/, const Mesh& /*mesh*/, gl::Uniform& uniform)
           { uniform.set(m_geometryTextures); });
  m->getUniform("u_isSprite")->set(0);

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
        uniform.set(m_csm->getTextures());
      });

  if(auto uniform = m->tryGetUniform("u_noise"))
    uniform->set(m_noiseTexture);

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

const std::shared_ptr<Material>& MaterialManager::getWaterSurface()
{
  if(m_waterSurface != nullptr)
    return m_waterSurface;

  m_waterSurface = std::make_shared<Material>(m_shaderCache->getWaterSurface());
  m_waterSurface->getRenderState().setCullFace(false);

  m_waterSurface->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  m_waterSurface->getUniform("u_time")->bind(
    [renderer = m_renderer](const Node&, const Mesh& /*mesh*/, gl::Uniform& uniform)
    {
      const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
      uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
    });

  m_waterSurface->getUniform("u_noise")->set(m_noiseTexture);

  return m_waterSurface;
}

const std::shared_ptr<Material>& MaterialManager::getLightning()
{
  if(m_lightning != nullptr)
    return m_lightning;

  m_lightning = std::make_shared<render::scene::Material>(m_shaderCache->getLightning());
  m_lightning->getUniformBlock("Transform")->bindTransformBuffer();
  m_lightning->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());

  return m_lightning;
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

  auto noiseTexture = std::make_shared<gl::Texture2D<gl::RGB8>>(glm::ivec2{NoiseTextureSize}, "noise");
  noiseTexture->assign(noiseData.data());
  auto sampler = std::make_unique<gl::Sampler>("noise");
  sampler->set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::MirroredRepeat)
    .set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::MirroredRepeat)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);
  m_noiseTexture = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::RGB8>>>(noiseTexture, std::move(sampler));
}

std::shared_ptr<Material>
  MaterialManager::getComposition(bool water, bool lensDistortion, bool dof, bool filmGrain, bool hbao, bool velvia)
{
  const std::tuple key{water, lensDistortion, dof, filmGrain, hbao, velvia};
  if(auto it = m_composition.find(key); it != m_composition.end())
    return it->second;
  auto m
    = std::make_shared<Material>(m_shaderCache->getComposition(water, lensDistortion, dof, filmGrain, hbao, velvia));

  if(auto uniform = m->tryGetUniform("u_time"))
  {
    uniform->bind(
      [renderer = m_renderer](const Node&, const Mesh& /*mesh*/, gl::Uniform& uniform)
      {
        const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
        uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
      });
  }

  if(auto uniform = m->tryGetUniform("distortion_power"))
    uniform->set(water ? -2.0f : -1.0f);

  if(auto uniform = m->tryGetUniform("u_noise"))
    uniform->set(m_noiseTexture);

  m_composition.emplace(key, m);
  return m;
}

const std::shared_ptr<Material>& MaterialManager::getCrt()
{
  if(m_crt != nullptr)
    return m_crt;

  auto m = std::make_shared<Material>(m_shaderCache->getCrt());
  m->getUniform("u_time")->bind(
    [renderer = m_renderer](const Node&, const Mesh& /*mesh*/, gl::Uniform& uniform)
    {
      const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
      uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
    });
  m->getUniform("u_noise")->set(m_noiseTexture);
  m_crt = m;
  return m_crt;
}

const std::shared_ptr<Material>& MaterialManager::getUi()
{
  if(m_ui != nullptr)
    return m_ui;

  auto m = std::make_shared<Material>(m_shaderCache->getUi());
  m->getUniform("u_input")->bind([this](const Node& /*node*/, const Mesh& /*mesh*/, gl::Uniform& uniform)
                                 { uniform.set(m_geometryTextures); });
  configureForScreenSpaceEffect(*m, true);
  m_ui = m;
  return m_ui;
}

std::shared_ptr<Material> MaterialManager::getFlat(bool withAlpha, bool invertY, bool withAspectRatio)
{
  const std::tuple key{withAlpha, invertY, withAspectRatio};
  if(auto it = m_flat.find(key); it != m_flat.end())
    return it->second;

  auto m = std::make_shared<Material>(m_shaderCache->getFlat(withAlpha, invertY, withAspectRatio));
  m->getRenderState().setBlend(withAlpha);
  m->getRenderState().setBlendFactors(gl::api::BlendingFactor::SrcAlpha, gl::api::BlendingFactor::OneMinusSrcAlpha);
  configureForScreenSpaceEffect(*m);
  m_flat.emplace(key, m);
  return m;
}

const std::shared_ptr<Material>& MaterialManager::getBackdrop()
{
  if(m_backdrop != nullptr)
    return m_backdrop;

  auto m = std::make_shared<Material>(m_shaderCache->getBackdrop());
  configureForScreenSpaceEffect(*m);
  m_backdrop = m;
  return m_backdrop;
}

const std::shared_ptr<Material>& MaterialManager::getFXAA()
{
  if(m_fxaa != nullptr)
    return m_fxaa;

  auto m = std::make_shared<Material>(m_shaderCache->getFXAA());
  configureForScreenSpaceEffect(*m);
  m_fxaa = m;
  return m_fxaa;
}

const std::shared_ptr<Material>& MaterialManager::getHBAO()
{
  if(m_hbao != nullptr)
    return m_hbao;

  auto m = std::make_shared<Material>(m_shaderCache->getHBAO());
  m->getUniform("u_noise")->set(m_noiseTexture);
  configureForScreenSpaceEffect(*m);
  m_hbao = m;
  return m_hbao;
}

const std::shared_ptr<Material>& MaterialManager::getLinearDepth()
{
  if(m_linearDepth != nullptr)
    return m_linearDepth;

  auto m = std::make_shared<Material>(m_shaderCache->getLinearDepth());
  configureForScreenSpaceEffect(*m);
  m_linearDepth = m;
  return m_linearDepth;
}

const std::shared_ptr<Material>& MaterialManager::getVSMSquare()
{
  if(m_vsmSquare != nullptr)
    return m_vsmSquare;

  auto m = std::make_shared<Material>(m_shaderCache->getVSMSquare());
  configureForScreenSpaceEffect(*m);
  m_vsmSquare = m;
  return m_vsmSquare;
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

  auto sampler = std::make_unique<gl::Sampler>("geometry-sampler");
  if(bilinear)
  {
    sampler->set(gl::api::TextureMinFilter::LinearMipmapLinear);
    sampler->set(gl::api::TextureMagFilter::Linear);
  }
  else
  {
    sampler->set(gl::api::TextureMinFilter::NearestMipmapLinear);
    sampler->set(gl::api::TextureMagFilter::Nearest);
  }

  if(anisotropyLevel != 0 && gl::hasAnisotropicFilteringExtension())
    sampler->set(gl::api::SamplerParameterF::TextureMaxAnisotropy, anisotropyLevel);

  m_geometryTextures = std::make_shared<gl::TextureHandle<gl::Texture2DArray<gl::SRGBA8>>>(
    m_geometryTextures->getTexture(), std::move(sampler));
}

std::shared_ptr<Material> MaterialManager::getFastGaussBlur(uint8_t extent, uint8_t blurDir, uint8_t blurDim)
{
  const std::tuple key{extent, blurDir, blurDim};
  if(auto it = m_fastGaussBlur.find(key); it != m_fastGaussBlur.end())
    return it->second;

  auto m = std::make_shared<Material>(m_shaderCache->getFastGaussBlur(extent, blurDim));
  configureForScreenSpaceEffect(*m);
  m->getUniform("u_blurDir")->set(int(blurDir));
  m_fastGaussBlur[key] = m;
  return m;
}

std::shared_ptr<Material> MaterialManager::getFastBoxBlur(uint8_t extent, uint8_t blurDir, uint8_t blurDim)
{
  const std::tuple key{extent, blurDir, blurDim};
  if(auto it = m_fastBoxBlur.find(key); it != m_fastBoxBlur.end())
    return it->second;

  auto m = std::make_shared<Material>(m_shaderCache->getFastBoxBlur(extent, blurDim));
  configureForScreenSpaceEffect(*m);
  m->getUniform("u_blurDir")->set(int(blurDir));
  m_fastBoxBlur[key] = m;
  return m;
}
} // namespace render::scene
