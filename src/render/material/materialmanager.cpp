#include "materialmanager.h"

#include "bufferparameter.h"
#include "material.h"
#include "render/scene/csm.h"
#include "render/scene/node.h"
#include "render/scene/renderer.h"
#include "shadercache.h"
#include "spritematerialmode.h"
#include "uniformparameter.h"

#include <array>
#include <boost/assert.hpp>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <gl/buffer.h>
#include <gl/constants.h>
#include <gl/glad_init.h>
#include <gl/pixel.h>
#include <gl/program.h>
#include <gl/renderstate.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texture2darray.h> // IWYU pragma: keep
#include <gl/texturehandle.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace render::scene
{
class Mesh;
}

namespace render::material
{
namespace
{
void configureForScreenSpaceEffect(Material& m, bool enableBlend)
{
  m.getRenderState().setDepthTest(false);
  m.getRenderState().setDepthWrite(false);
  m.getRenderState().setBlend(0, enableBlend);
  if(enableBlend)
    m.getRenderState().setBlendFactors(0, gl::api::BlendingFactor::One, gl::api::BlendingFactor::OneMinusSrcAlpha);
}
} // namespace

struct alignas(sizeof(uint64_t)) MaterialManager::CsmHandleContainer
{
  std::array<glm::uvec4, render::scene::CSMBuffer::NSplits> handles;
};

gslu::nn_shared<Material> MaterialManager::getSprite(SpriteMaterialMode mode,
                                                     const std::function<int32_t()>& lightingMode)
{
  if(auto it = m_sprite.find(mode); it != m_sprite.end())
    return it->second;

  auto m
    = gsl::make_shared<Material>(m_shaderCache->getGeometry(false, false, true, false, static_cast<uint8_t>(mode)));
  m->getRenderState().setCullFace(false);

  if(mode != SpriteMaterialMode::InstancedBillboard)
    m->getUniformBlock("Transform")->bindTransformBuffer();
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  m->getUniform("u_diffuseTextures")
    ->bind(
      [this](const scene::Node* /*node*/, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
      {
        uniform.set(gsl::not_null{m_geometryTexturesHandle});
      });
  m->getUniform("u_lightingMode")
    ->bind(
      [lightingMode](const scene::Node* /*node*/, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
      {
        uniform.set(lightingMode());
      });

  m_sprite.emplace(mode, m);
  return m;
}

gslu::nn_shared<Material> MaterialManager::getCSMDepthOnly(bool skeletal, const std::function<bool()>& smooth)
{
  auto m = gsl::make_shared<Material>(m_shaderCache->getCSMDepthOnly(skeletal));
  m->getUniform("u_mvp")->bind(
    [this](const scene::Node* node, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
    {
      BOOST_ASSERT(node != nullptr);
      BOOST_ASSERT(m_csm != nullptr);
      uniform.set(m_csm->getActiveMatrix(node->getModelMatrix()));
    });
  m->getRenderState().setDepthTest(true);
  m->getRenderState().setDepthWrite(true);
  m->getRenderState().setDepthClamp(true);
  m->getRenderState().setCullFaceSide(gl::api::TriangleFace::Front);
  if(auto buffer = m->tryGetBuffer("BoneTransform"))
    buffer->bindBoneTransformBuffer(smooth);

  return m;
}

gslu::nn_shared<Material> MaterialManager::getDepthOnly(bool skeletal, const std::function<bool()>& smooth)
{
  auto m = gsl::make_shared<Material>(m_shaderCache->getDepthOnly(skeletal));
  m->getRenderState().setDepthTest(true);
  m->getRenderState().setDepthWrite(true);
  m->getUniformBlock("Transform")->bindTransformBuffer();
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  if(auto buffer = m->tryGetBuffer("BoneTransform"))
    buffer->bindBoneTransformBuffer(smooth);
  m->getUniform("u_diffuseTextures")
    ->bind(
      [this](const scene::Node* /*node*/, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
      {
        uniform.set(gsl::not_null{m_geometryTexturesHandle});
      });

  return m;
}

gslu::nn_shared<Material> MaterialManager::getGeometry(bool inWater,
                                                       bool skeletal,
                                                       bool roomShadowing,
                                                       bool opaque,
                                                       const std::function<bool()>& smooth,
                                                       const std::function<int32_t()>& lightingMode)
{
  gsl_Expects(m_geometryTexturesHandle != nullptr);

  auto m = gsl::make_shared<Material>(m_shaderCache->getGeometry(inWater, skeletal, roomShadowing, opaque, 0));
  m->getUniform("u_diffuseTextures")
    ->bind(
      [this](const scene::Node* /*node*/, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
      {
        uniform.set(gsl::not_null{m_geometryTexturesHandle});
      });

  m->getUniformBlock("Transform")->bindTransformBuffer();
  if(auto buffer = m->tryGetBuffer("BoneTransform"))
    buffer->bindBoneTransformBuffer(smooth);
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  m->getUniformBlock("CSM")->bind(
    [this](const scene::Node* node, const scene::Mesh& /*mesh*/, gl::UniformBlock& ub)
    {
      BOOST_ASSERT(node != nullptr);
      BOOST_ASSERT(m_csm != nullptr);
      ub.bind(m_csm->getBuffer(node->getModelMatrix()));
    });

  m->getUniformBlock("CSMVSM")->bind(
    [this](const render::scene::Node*, const render::scene::Mesh& /*mesh*/, gl::UniformBlock& uniformBlock)
    {
      BOOST_ASSERT(m_csm != nullptr);
      if(m_csmBuffer == nullptr)
        m_csmBuffer = std::make_shared<gl::UniformBuffer<CsmHandleContainer>>(
          "csm-handles", gl::api::BufferUsage::StreamDraw, sizeof(CsmHandleContainer));
      CsmHandleContainer data{};
      auto textures = m_csm->getTextures();
      for(size_t i = 0; i < render::scene::CSMBuffer::NSplits; ++i)
        *reinterpret_cast<uint64_t*>(&data.handles[i]) = textures[i]->getHandle();
      m_csmBuffer->setSubData(data, 0);
      uniformBlock.bind(*m_csmBuffer);
    });

  m->getUniform("u_lightingMode")
    ->bind(
      [lightingMode](const scene::Node* /*node*/, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
      {
        uniform.set(lightingMode());
      });

  if(auto uniform = m->tryGetUniform("u_noise"))
    uniform->set(gsl::not_null{m_noiseTexture});

  if(auto uniform = m->tryGetUniform("u_time"))
  {
    uniform->bind(
      [renderer = m_renderer](const scene::Node*, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
      {
        const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
        uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
      });
  }

  return m;
}

gslu::nn_shared<Material> MaterialManager::getGhost(const std::function<bool()>& smooth)
{
  if(m_ghost != nullptr)
    return gsl::not_null{m_ghost};

  auto m = gsl::make_shared<Material>(m_shaderCache->getGhost());
  m->getUniformBlock("Transform")->bindTransformBuffer();
  m->getBuffer("BoneTransform")->bindBoneTransformBuffer(smooth);
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  m_ghost = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getWaterSurface()
{
  if(m_waterSurface != nullptr)
    return gsl::not_null{m_waterSurface};

  m_waterSurface = std::make_shared<Material>(m_shaderCache->getWaterSurface());
  m_waterSurface->getRenderState().setCullFace(false);
  m_waterSurface->getRenderState().setBlend(0, false);
  m_waterSurface->getRenderState().setBlend(1, false);

  m_waterSurface->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  m_waterSurface->getUniform("u_time")->bind(
    [renderer = m_renderer](const scene::Node*, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
    {
      const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
      uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
    });

  m_waterSurface->getUniform("u_noise")->set(gsl::not_null{m_noiseTexture});

  return gsl::not_null{m_waterSurface};
}

gslu::nn_shared<Material> MaterialManager::getLightning()
{
  if(m_lightning != nullptr)
    return gsl::not_null{m_lightning};

  m_lightning = std::make_shared<Material>(m_shaderCache->getLightning());
  m_lightning->getUniformBlock("Transform")->bindTransformBuffer();
  m_lightning->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());

  return gsl::not_null{m_lightning};
}

MaterialManager::MaterialManager(gslu::nn_shared<ShaderCache> shaderCache, gslu::nn_shared<scene::Renderer> renderer)
    : m_shaderCache{std::move(shaderCache)}
    , m_renderer{std::move(renderer)}
{
  static constexpr size_t NoiseTextureSize = 128;
  std::vector<gl::RGB8> noiseData;
  noiseData.resize(NoiseTextureSize * NoiseTextureSize);
  std::default_random_engine generator{}; // NOLINT(cert-msc32-c, cert-msc51-cpp)
  std::uniform_int_distribution<uint16_t> randomInts(0, 255);
  for(auto& i : noiseData)
  {
    const auto value = randomInts(generator);
    i = gl::RGB8{gsl::narrow_cast<uint8_t>(value), gsl::narrow_cast<uint8_t>(value), gsl::narrow_cast<uint8_t>(value)};
  }

  auto noiseTexture = gsl::make_shared<gl::Texture2D<gl::RGB8>>(glm::ivec2{NoiseTextureSize}, "noise");
  noiseTexture->assign(noiseData);
  m_noiseTexture = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::RGB8>>>(
    noiseTexture,
    gsl::make_unique<gl::Sampler>("noise" + gl::SamplerSuffix)
      | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::MirroredRepeat)
      | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::MirroredRepeat)
      | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear));
}

gslu::nn_shared<Material> MaterialManager::getWorldComposition(bool inWater, bool dof)
{
  const std::tuple key{inWater, dof};
  if(auto it = m_composition.find(key); it != m_composition.end())
    return it->second;
  auto m = gsl::make_shared<Material>(m_shaderCache->getWorldComposition(inWater, dof));

  if(auto uniform = m->tryGetUniform("u_time"))
  {
    uniform->bind(
      [renderer = m_renderer](const scene::Node*, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
      {
        const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
        uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
      });
  }

  if(auto uniform = m->tryGetUniform("u_noise"))
    uniform->set(gsl::not_null{m_noiseTexture});

  configureForScreenSpaceEffect(*m, false);

  m_composition.emplace(key, m);
  return m;
}

gslu::nn_shared<Material> MaterialManager::getUi()
{
  if(m_ui != nullptr)
    return gsl::not_null{m_ui};

  auto m = std::make_shared<Material>(m_shaderCache->getUi());
  m->getUniform("u_input")->bind(
    [this](const scene::Node* /*node*/, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
    {
      uniform.set(gsl::not_null{m_geometryTexturesHandle});
    });
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  configureForScreenSpaceEffect(*m, true);
  m_ui = m;
  return gsl::not_null{m_ui};
}

gslu::nn_shared<Material> MaterialManager::getFlat(bool withAlpha, bool invertY, bool withAspectRatio)
{
  const std::tuple key{withAlpha, invertY, withAspectRatio};
  if(auto it = m_flat.find(key); it != m_flat.end())
    return it->second;

  auto m = gsl::make_shared<Material>(m_shaderCache->getFlat(withAlpha, invertY, withAspectRatio));
  m->getRenderState().setBlend(0, withAlpha);
  m->getRenderState().setBlendFactors(0, gl::api::BlendingFactor::One, gl::api::BlendingFactor::OneMinusSrcAlpha);
  if(const auto uniformBlock = m->tryGetUniformBlock("Camera"))
    uniformBlock->bindCameraBuffer(m_renderer->getCamera());
  configureForScreenSpaceEffect(*m, withAlpha);
  m_flat.emplace(key, m);
  return m;
}

gslu::nn_shared<Material> MaterialManager::getBackdrop(bool withAlphaMultiplier)
{
  if(auto it = m_backdrop.find(withAlphaMultiplier); it != m_backdrop.end())
    return it->second;

  auto m = gsl::make_shared<Material>(m_shaderCache->getBackdrop(withAlphaMultiplier));
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  configureForScreenSpaceEffect(*m, withAlphaMultiplier);
  m_backdrop.emplace(withAlphaMultiplier, m);
  return m;
}

gslu::nn_shared<Material> MaterialManager::getFXAA(uint8_t preset)
{
  if(auto it = m_fxaa.find(preset); it != m_fxaa.end())
    return it->second;

  auto m = gsl::make_shared<Material>(m_shaderCache->getFXAA(preset));
  configureForScreenSpaceEffect(*m, false);
  m_fxaa.emplace(preset, m);
  return m;
}

gslu::nn_shared<Material> MaterialManager::getCRTV0()
{
  if(m_crtV0 != nullptr)
    return gsl::not_null{m_crtV0};

  auto m = gsl::make_shared<Material>(m_shaderCache->getCRTV0());
  m->getUniform("u_noise")->set(gsl::not_null{m_noiseTexture});
  configureForScreenSpaceEffect(*m, false);
  m_crtV0 = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getCRTV1()
{
  if(m_crtV1 != nullptr)
    return gsl::not_null{m_crtV1};

  auto m = gsl::make_shared<Material>(m_shaderCache->getCRTV1());
  configureForScreenSpaceEffect(*m, false);
  m_crtV1 = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getBrightnessContrast(int8_t brightness, int8_t contrast)
{
  const std::tuple<int8_t, int8_t> key{brightness, contrast};
  if(auto it = m_brightnessContrast.find(key); it != m_brightnessContrast.end())
    return it->second;

  auto m = gsl::make_shared<Material>(m_shaderCache->getBrightnessContrast(brightness, contrast));
  configureForScreenSpaceEffect(*m, false);
  m_brightnessContrast.emplace(key, m);
  return m;
}

gslu::nn_shared<Material> MaterialManager::getVelvia()
{
  if(m_velvia != nullptr)
    return gsl::not_null{m_velvia};

  auto m = gsl::make_shared<Material>(m_shaderCache->getVelvia());
  configureForScreenSpaceEffect(*m, false);
  m_velvia = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getDeath()
{
  if(m_death != nullptr)
    return gsl::not_null{m_death};

  auto m = gsl::make_shared<Material>(m_shaderCache->getDeath());
  configureForScreenSpaceEffect(*m, false);
  m_death = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getFilmGrain()
{
  if(m_filmGrain != nullptr)
    return gsl::not_null{m_filmGrain};

  auto m = gsl::make_shared<Material>(m_shaderCache->getFilmGrain());
  m->getUniform("u_noise")->set(gsl::not_null{m_noiseTexture});
  configureForScreenSpaceEffect(*m, false);
  m_filmGrain = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getLensDistortion()
{
  if(m_lensDistortion != nullptr)
    return gsl::not_null{m_lensDistortion};

  auto m = gsl::make_shared<Material>(m_shaderCache->getLensDistortion());
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());

  m_lensDistortion = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getMasking(bool ao, bool edges)
{
  const std::tuple key{ao, edges};
  if(auto it = m_masking.find(key); it != m_masking.end())
    return it->second;

  auto m = gsl::make_shared<Material>(m_shaderCache->getMasking(ao, edges));
  m_masking.emplace(key, m);
  return m;
}

gslu::nn_shared<Material> MaterialManager::getUnderwaterMovement()
{
  if(m_underwaterMovement != nullptr)
    return gsl::not_null{m_underwaterMovement};

  auto m = gsl::make_shared<Material>(m_shaderCache->getUnderwaterMovement());
  m_underwaterMovement = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getReflective()
{
  if(m_reflective != nullptr)
    return gsl::not_null{m_reflective};

  auto m = gsl::make_shared<Material>(m_shaderCache->getReflective());
  configureForScreenSpaceEffect(*m, false);
  m_reflective = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getBloom()
{
  if(m_bloom != nullptr)
    return gsl::not_null{m_bloom};

  auto m = gsl::make_shared<Material>(m_shaderCache->getBloom());
  configureForScreenSpaceEffect(*m, false);
  m_bloom = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getFilmNoir()
{
  if(m_filmNoir != nullptr)
    return gsl::not_null{m_filmNoir};

  auto m = gsl::make_shared<Material>(m_shaderCache->getFilmNoir());
  m->getUniform("u_time")->bind(
    [renderer = m_renderer](const scene::Node*, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
    {
      const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
      uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
    });
  configureForScreenSpaceEffect(*m, false);
  m_filmNoir = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getHBAO()
{
  if(m_hbao != nullptr)
    return gsl::not_null{m_hbao};

  auto m = gsl::make_shared<Material>(m_shaderCache->getHBAO());
  m->getUniform("u_noise")->set(gsl::not_null{m_noiseTexture});
  configureForScreenSpaceEffect(*m, false);
  m_hbao = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getEdgeDetection()
{
  if(m_edgeDetection != nullptr)
    return gsl::not_null{m_edgeDetection};

  auto m = gsl::make_shared<Material>(m_shaderCache->getEdgeDetection());
  configureForScreenSpaceEffect(*m, false);
  m_edgeDetection = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getEdgeDilation()
{
  if(m_edgeDilation != nullptr)
    return gsl::not_null{m_edgeDilation};

  auto m = gsl::make_shared<Material>(m_shaderCache->getEdgeDilation());
  configureForScreenSpaceEffect(*m, false);
  m_edgeDilation = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getVSMSquare()
{
  if(m_vsmSquare != nullptr)
    return gsl::not_null{m_vsmSquare};

  auto m = gsl::make_shared<Material>(m_shaderCache->getVSMSquare());
  configureForScreenSpaceEffect(*m, false);
  m_vsmSquare = m;
  return m;
}

void MaterialManager::setGeometryTextures(
  const gslu::nn_shared<gl::Texture2DArray<gl::PremultipliedSRGBA8>>& geometryTextures)
{
  createSampler(geometryTextures, false, std::nullopt);
}

void MaterialManager::setFiltering(bool bilinear, const std::optional<float>& anisotropyLevel)
{
  if(m_geometryTexturesHandle == nullptr)
    return;

  createSampler(m_geometryTexturesHandle->getTexture(), bilinear, anisotropyLevel);
}

gslu::nn_shared<Material> MaterialManager::getFastGaussBlur(uint8_t extent, uint8_t blurDir, uint8_t blurDim)
{
  const std::tuple key{extent, blurDir, blurDim};
  if(auto it = m_fastGaussBlur.find(key); it != m_fastGaussBlur.end())
    return it->second;

  auto m = gsl::make_shared<Material>(m_shaderCache->getFastGaussBlur(extent, blurDim));
  configureForScreenSpaceEffect(*m, false);
  m->getUniform("u_blurDir")->set(int(blurDir));
  m_fastGaussBlur.emplace(key, m);
  return m;
}

gslu::nn_shared<Material> MaterialManager::getFastBoxBlur(uint8_t extent, uint8_t blurDir, uint8_t blurDim)
{
  const std::tuple key{extent, blurDir, blurDim};
  if(auto it = m_fastBoxBlur.find(key); it != m_fastBoxBlur.end())
    return it->second;

  auto m = gsl::make_shared<Material>(m_shaderCache->getFastBoxBlur(extent, blurDim));
  configureForScreenSpaceEffect(*m, false);
  m->getUniform("u_blurDir")->set(int(blurDir));
  m_fastBoxBlur.emplace(key, m);
  return m;
}

gslu::nn_shared<Material> MaterialManager::getBloomDownsample()
{
  if(m_bloomDownsample != nullptr)
    return gsl::not_null{m_bloomDownsample};

  auto m = gsl::make_shared<Material>(m_shaderCache->getBloomDownsample());
  configureForScreenSpaceEffect(*m, false);
  m_bloomDownsample = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getBloomUpsample()
{
  if(m_bloomUpsample != nullptr)
    return gsl::not_null{m_bloomUpsample};

  auto m = gsl::make_shared<Material>(m_shaderCache->getBloomUpsample());
  configureForScreenSpaceEffect(*m, false);
  m_bloomUpsample = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getDustParticle()
{
  if(m_dustParticle != nullptr)
    return gsl::not_null{m_dustParticle};

  auto m = gsl::make_shared<Material>(m_shaderCache->getDustParticle());
  m->getRenderState().setCullFace(false);
  m->getRenderState().setBlend(0, true);
  m->getRenderState().setBlendFactors(0, gl::api::BlendingFactor::One, gl::api::BlendingFactor::OneMinusSrcAlpha);
  m->getRenderState().setDepthTest(true);
  m->getRenderState().setDepthWrite(false);
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  m->getUniformBlock("Transform")->bindTransformBuffer();
  m->getUniform("u_noise")->set(gsl::not_null{m_noiseTexture});
  m->getUniform("u_time")->bind(
    [renderer = m_renderer](const scene::Node*, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
    {
      const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
      uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
    });
  m_dustParticle = m;
  return m;
}

gslu::nn_shared<Material> MaterialManager::getGhostName()
{
  if(m_ghostName != nullptr)
    return gsl::not_null{m_ghostName};

  auto m = gsl::make_shared<Material>(m_shaderCache->getGhostName());
  auto& renderState = m->getRenderState();
  renderState.setCullFace(false);
  renderState.setBlend(0, true);
  renderState.setBlendFactors(0, gl::api::BlendingFactor::One, gl::api::BlendingFactor::OneMinusSrcAlpha);
  renderState.setDepthTest(true);
  // TODO this seems wrong, need to verify
  renderState.setDepthWrite(false);
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  m->getUniformBlock("Transform")->bindTransformBuffer();
  m_ghostName = m;
  return m;
}

void MaterialManager::setDeathStrength(float strength)
{
  auto m = getDeath();
  m->getUniform("u_strength")->set(strength);
}

void MaterialManager::createSampler(
  const gslu::nn_shared<gl::Texture2DArray<gl::PremultipliedSRGBA8>>& geometryTextures,
  bool bilinear,
  const std::optional<float>& anisotropyLevel)
{
  auto sampler = gsl::make_unique<gl::Sampler>("geometry")
                 | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
                 | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge);
  if(bilinear)
  {
    sampler->set(gl::api::TextureMinFilter::LinearMipmapLinear).set(gl::api::TextureMagFilter::Linear);
  }
  else
  {
    sampler->set(gl::api::TextureMinFilter::NearestMipmapLinear).set(gl::api::TextureMagFilter::Nearest);
  }

  if(anisotropyLevel.has_value() && gl::hasAnisotropicFilteringExtension())
    sampler->set(gl::api::SamplerParameterF::TextureMaxAnisotropy, *anisotropyLevel);

  m_geometryTexturesHandle = std::make_shared<gl::TextureHandle<gl::Texture2DArray<gl::PremultipliedSRGBA8>>>(
    geometryTextures, std::move(sampler));
}
} // namespace render::material
