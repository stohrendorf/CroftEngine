#include "materialmanager.h"

#include "csm.h"
#include "node.h"
#include "renderer.h"
#include "shadercache.h"

#include <gl/texture2darray.h>
#include <random>
#include <utility>

namespace render::scene
{
namespace
{
void configureForScreenSpaceEffect(Material& m, bool enableBlend = false)
{
  m.getRenderState().setDepthTest(false);
  m.getRenderState().setDepthWrite(false);
  if(enableBlend)
  {
    m.getRenderState().setBlend(true);
    m.getRenderState().setBlendSrc(gl::api::BlendingFactor::SrcAlpha);
    m.getRenderState().setBlendDst(gl::api::BlendingFactor::OneMinusSrcAlpha);
  }
}
} // namespace

const std::shared_ptr<Material>& MaterialManager::getSprite()
{
  if(m_sprite != nullptr)
    return m_sprite;

  m_sprite = std::make_shared<Material>(m_shaderCache->getGeometry(false, false, true));
  m_sprite->getRenderState().setCullFace(false);

  m_sprite->getUniformBlock("Transform")->bindTransformBuffer();
  m_sprite->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());

  return m_sprite;
}

const std::shared_ptr<Material>& MaterialManager::getCSMDepthOnly(bool skeletal)
{
  if(const auto& tmp = m_csmDepthOnly[skeletal])
    return tmp;

  m_csmDepthOnly[skeletal] = std::make_shared<Material>(m_shaderCache->getCSMDepthOnly(skeletal));
  m_csmDepthOnly[skeletal]->getUniform("u_mvp")->bind(
    [this](const Node& node, const Mesh& /*mesh*/, gl::Uniform& uniform) {
      BOOST_ASSERT(m_csm != nullptr);
      uniform.set(m_csm->getActiveMatrix(node.getModelMatrix()));
    });
  m_csmDepthOnly[skeletal]->getRenderState().setDepthTest(true);
  m_csmDepthOnly[skeletal]->getRenderState().setDepthWrite(true);
  if(skeletal)
    m_csmDepthOnly[skeletal]->getBuffer("BoneTransform")->bindBoneTransformBuffer();

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
  if(skeletal)
    m_depthOnly[skeletal]->getBuffer("BoneTransform")->bindBoneTransformBuffer();
  m_depthOnly[skeletal]->getUniform("u_diffuseTextures")->set(m_geometryTextures);

  return m_depthOnly[skeletal];
}

std::shared_ptr<Material> MaterialManager::getGeometry(bool water, bool skeletal, bool roomShadowing)
{
  Expects(m_geometryTextures != nullptr);
  const std::tuple key{water, skeletal, roomShadowing};
  if(auto it = m_geometry.find(key); it != m_geometry.end())
    return it->second;

  auto m = std::make_shared<Material>(m_shaderCache->getGeometry(water, skeletal, roomShadowing));
  m->getUniform("u_diffuseTextures")->set(m_geometryTextures);
  m->getUniform("u_spritePole")->set(-1);

  m->getUniformBlock("Transform")->bindTransformBuffer();
  if(skeletal)
    m->getBuffer("BoneTransform")->bindBoneTransformBuffer();
  m->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  m->getUniformBlock("CSM")->bind([this](const Node& node, const Mesh& /*mesh*/, gl::UniformBlock& ub) {
    BOOST_ASSERT(m_csm != nullptr);
    ub.bind(m_csm->getBuffer(node.getModelMatrix()));
  });

  BOOST_ASSERT(m_csm != nullptr);
  m->getUniform("u_csmVsm[0]")->set(m_csm->getTextures());

  if(water)
    m->getUniform("u_noise")->set(m_noiseTexture);

  if(water)
  {
    m->getUniform("u_time")->bind([renderer = m_renderer](const Node&, const Mesh& /*mesh*/, gl::Uniform& uniform) {
      const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
      uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
    });
  }

  m_geometry.emplace(key, m);
  return m;
}

const std::shared_ptr<Material>& MaterialManager::getPortal()
{
  if(m_portal != nullptr)
    return m_portal;

  m_portal = std::make_shared<Material>(m_shaderCache->getPortal());
  m_portal->getRenderState().setCullFace(false);

  m_portal->getUniformBlock("Camera")->bindCameraBuffer(m_renderer->getCamera());
  m_portal->getUniform("u_time")->bind(
    [renderer = m_renderer](const Node&, const Mesh& /*mesh*/, gl::Uniform& uniform) {
      const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
      uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
    });

  m_portal->getUniform("u_noise")->set(m_noiseTexture);

  return m_portal;
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

  m_noiseTexture = std::make_shared<gl::Texture2D<gl::RGB8>>(glm::ivec2{NoiseTextureSize}, "noise");
  m_noiseTexture->assign(noiseData.data())
    .set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::MirroredRepeat)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::MirroredRepeat)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);
}

std::shared_ptr<Material> MaterialManager::getComposition(bool water, bool lensDistortion, bool dof, bool filmGrain)
{
  const std::tuple key{water, lensDistortion, dof, filmGrain};
  if(auto it = m_composition.find(key); it != m_composition.end())
    return it->second;
  auto m = std::make_shared<Material>(m_shaderCache->getComposition(water, lensDistortion, dof, filmGrain));

  if(m->getShaderProgram()->findUniform("u_time") != nullptr)
    m->getUniform("u_time")->bind([renderer = m_renderer](const Node&, const Mesh& /*mesh*/, gl::Uniform& uniform) {
      const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
      uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
    });

  if(lensDistortion)
    m->getUniform("distortion_power")->set(water ? -2.0f : -1.0f);

  m->getUniform("u_noise")->set(m_noiseTexture);

  m_composition.emplace(key, m);
  return m;
}

const std::shared_ptr<Material>& MaterialManager::getCrt()
{
  if(m_crt != nullptr)
    return m_crt;

  auto m = std::make_shared<Material>(m_shaderCache->getCrt());
  m->getUniform("u_time")->bind([renderer = m_renderer](const Node&, const Mesh& /*mesh*/, gl::Uniform& uniform) {
    const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(renderer->getGameTime());
    uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
  });
  m->getUniform("u_noise")->set(m_noiseTexture);
  m_crt = m;
  return m_crt;
}

const std::shared_ptr<Material>& MaterialManager::getScreenSpriteTextured()
{
  if(m_screenSpriteTextured != nullptr)
    return m_screenSpriteTextured;

  auto m = std::make_shared<Material>(m_shaderCache->getScreenSpriteTextured());
  m->getUniform("u_input")->set(m_geometryTextures);
  configureForScreenSpaceEffect(*m, true);
  m_screenSpriteTextured = m;
  return m_screenSpriteTextured;
}

const std::shared_ptr<Material>& MaterialManager::getScreenSpriteColorRect()
{
  if(m_screenSpriteColorRect != nullptr)
    return m_screenSpriteColorRect;

  auto m = std::make_shared<Material>(m_shaderCache->getScreenSpriteColorRect());
  configureForScreenSpaceEffect(*m, true);
  m_screenSpriteColorRect = m;
  return m_screenSpriteColorRect;
}

std::shared_ptr<Material> MaterialManager::getFlat(bool withAlpha, bool invertY)
{
  const std::tuple key{withAlpha, invertY};
  if(auto it = m_flat.find(key); it != m_flat.end())
    return it->second;

  auto m = std::make_shared<Material>(m_shaderCache->getFlat(withAlpha, invertY));
  m->getRenderState().setBlend(withAlpha);
  m->getRenderState().setBlendSrc(gl::api::BlendingFactor::SrcAlpha);
  m->getRenderState().setBlendDst(gl::api::BlendingFactor::OneMinusSrcAlpha);
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

const std::shared_ptr<Material>& MaterialManager::getSSAO()
{
  if(m_ssao != nullptr)
    return m_ssao;

  auto m = std::make_shared<Material>(m_shaderCache->getSSAO());
  m->getUniform("u_noise")->set(m_noiseTexture);
  configureForScreenSpaceEffect(*m);
  m_ssao = m;
  return m_ssao;
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

void MaterialManager::setGeometryTextures(std::shared_ptr<gl::Texture2DArray<gl::SRGBA8>> geometryTextures)
{
  m_geometryTextures = std::move(geometryTextures);
  for(const auto& [_, mtl] : m_geometry)
    if(mtl != nullptr)
      mtl->getUniform("u_diffuseTextures")->set(m_geometryTextures);
  if(m_screenSpriteTextured != nullptr)
    m_screenSpriteTextured->getUniform("u_input")->set(m_geometryTextures);

  for(const auto& [_, mtl] : m_depthOnly)
    if(mtl != nullptr)
      mtl->getUniform("u_diffuseTextures")->set(m_geometryTextures);
}

void MaterialManager::setBilinearFiltering(bool enabled)
{
  if(m_geometryTextures == nullptr)
    return;

  if(enabled)
  {
    m_geometryTextures->set(gl::api::TextureMinFilter::LinearMipmapLinear);
    m_geometryTextures->set(gl::api::TextureMagFilter::Linear);
  }
  else
  {
    m_geometryTextures->set(gl::api::TextureMinFilter::NearestMipmapLinear);
    m_geometryTextures->set(gl::api::TextureMagFilter::Nearest);
  }
}

std::shared_ptr<Material> MaterialManager::getBlur(uint8_t extent, uint8_t blurDir, uint8_t blurDim, bool gauss)
{
  const std::tuple key{extent, blurDir, blurDim, gauss};
  if(auto it = m_blur.find(key); it != m_blur.end())
    return it->second;

  auto m = std::make_shared<Material>(m_shaderCache->getBlur(extent, blurDir, blurDim, gauss));
  configureForScreenSpaceEffect(*m);
  m_blur[key] = m;
  return m;
}
} // namespace render::scene
