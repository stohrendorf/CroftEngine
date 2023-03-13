#include "presenter.h"

#include "audio/soundengine.h"
#include "audiosettings.h"
#include "cameracontroller.h"
#include "core/i18n.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "objectmanager.h"
#include "objects/laraobject.h"
#include "objects/objectstate.h"
#include "qs/qs.h"
#include "render/material/material.h"
#include "render/material/materialgroup.h"
#include "render/material/materialmanager.h"
#include "render/material/rendermode.h"
#include "render/material/shadercache.h"
#include "render/material/uniformparameter.h"
#include "render/pass/config.h"
#include "render/renderpipeline.h"
#include "render/rendersettings.h"
#include "render/scene/camera.h"
#include "render/scene/csm.h"
#include "render/scene/mesh.h"
#include "render/scene/node.h"
#include "render/scene/renderable.h"
#include "render/scene/rendercontext.h"
#include "render/scene/renderer.h"
#include "render/scene/screenoverlay.h"
#include "render/scene/visitor.h"
#include "ui/text.h"
#include "ui/ui.h"
#include "util/helpers.h"
#include "video/videoplayer.h"
#include "world/room.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <gl/cimgwrapper.h>
#include <gl/constants.h>
#include <gl/debuggroup.h>
#include <gl/font.h>
#include <gl/framebuffer.h>
#include <gl/glassert.h>
#include <gl/glfw.h>
#include <gl/image.h>
#include <gl/pixel.h>
#include <gl/program.h>
#include <gl/renderstate.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturedepth.h>
#include <gl/texturehandle.h>
#include <gl/window.h>
#include <glm/common.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <gslu.h>
#include <limits>
#include <optional>
#include <utility>

namespace
{
constexpr int StatusLineFontSize = 40;

constexpr auto HealthChangeDuration = 1_sec * core::FrameRate;
constexpr auto HealthChangeDeltaPerFrame = core::LaraHealth * 1_frame / HealthChangeDuration;

constexpr auto HealthPulseDurationSlow = 3_sec * core::FrameRate;
constexpr auto HealthPulseDurationFast = 1_sec * core::FrameRate / 2;
constexpr auto HealthPulseMinHealth = core::LaraHealth / 5;
constexpr auto HealthPulseMaxHealth = core::LaraHealth / 2;
} // namespace

namespace engine
{
void Presenter::playVideo(const std::filesystem::path& path)
{
  util::ensureFileExists(path);

  auto mesh = render::scene::createScreenQuad(
    m_materialManager->getFlat(false, true, true), render::scene::Translucency::Opaque, "video");

  auto colorBuffer = gsl::make_shared<gl::Texture2D<gl::SRGB8>>(getRenderViewport(), "ui-color");
  auto fb = gl::FrameBufferBuilder()
              .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, colorBuffer)
              .build("video-fb");

  video::play(path,
              m_soundEngine->getDevice(),
              [&](const gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>& textureHandle)
              {
                if(!preFrame())
                  return true;

                m_renderer->getCamera()->setViewport(getRenderViewport());
                mesh->bind("u_input",
                           [&textureHandle](const render::scene::Node* /*node*/,
                                            const render::scene::Mesh& /*mesh*/,
                                            gl::Uniform& uniform)
                           {
                             uniform.set(textureHandle);
                           });
                mesh->getMaterialGroup()
                  .get(render::material::RenderMode::FullOpaque)
                  ->getUniformBlock("Camera")
                  ->bindCameraBuffer(m_renderer->getCamera());
                mesh->getRenderState().setViewport(getRenderViewport());

                fb->bind();
                {
                  render::scene::RenderContext context{
                    render::material::RenderMode::FullOpaque, std::nullopt, render::scene::Translucency::Opaque};
                  mesh->render(nullptr, context);
                }
                updateSoundEngine();
                fb->blit(*m_renderPipeline->getBackbuffer());
                swapBuffers();
                return !m_window->windowShouldClose() && !m_inputHandler->hasDebouncedAction(hid::Action::Menu);
              });
}

void Presenter::renderWorld(const std::vector<world::Room>& rooms,
                            const CameraController& cameraController,
                            const std::unordered_set<const world::Portal*>& waterEntryPortals,
                            const engine::world::World& world)
{
  m_renderPipeline->updateCamera(m_renderer->getCamera());

  {
    SOGLB_DEBUGGROUP("csm-pass");
    gl::RenderState::resetWantedState();
    gl::RenderState::getWantedState().setDepthClamp(true);
    m_csm->updateCamera(*m_renderer->getCamera());

    for(const auto& texture : m_csm->getDepthTextures())
      texture->clear(gl::ScalarDepth{1.0f});
    for(size_t i = 0; i < render::scene::CSMBuffer::NSplits; ++i)
    {
      SOGLB_DEBUGGROUP("csm-pass/" + std::to_string(i));

      m_csm->setActiveSplit(i);
      m_csm->getActiveFramebuffer()->bind();
      gl::RenderState::getWantedState() = m_csm->getActiveFramebuffer()->getRenderState();

      for(const auto translucencySelector :
          {render::scene::Translucency::Opaque, render::scene::Translucency::NonOpaque})
      {
        render::scene::RenderContext context{
          render::material::RenderMode::CSMDepthOnly, m_csm->getActiveMatrix(glm::mat4{1.0f}), translucencySelector};
        render::scene::Visitor visitor{context, false};
        for(const auto& room : rooms)
        {
          if(!room.node->isVisible())
            continue;

          for(const auto& child : room.node->getChildren())
          {
            visitor.visit(*child);
          }
        }
        visitor.render(glm::vec3{0.0f, 0.0f, std::numeric_limits<float>::lowest()});
        break;
      }
      m_csm->beginActiveDepthSync();
    }

    // ensure sync commands are flushed to the command queue
    GL_ASSERT(gl::api::flush());

    for(size_t i = 0; i < render::scene::CSMBuffer::NSplits; ++i)
    {
      SOGLB_DEBUGGROUP("csm-pass-square/" + std::to_string(i));
      m_csm->setActiveSplit(i);
      m_csm->waitActiveDepthSync();
      m_csm->renderSquare();
      m_csm->beginActiveSquareSync();
    }

    // ensure sync commands are flushed to the command queue
    GL_ASSERT(gl::api::flush());

    for(size_t i = 0; i < render::scene::CSMBuffer::NSplits; ++i)
    {
      SOGLB_DEBUGGROUP("csm-pass-blur/" + std::to_string(i));
      m_csm->setActiveSplit(i);
      m_csm->waitActiveSquareSync();
      m_csm->renderBlur();
      m_csm->beginActiveBlurSync();
    }

    // ensure sync commands are flushed to the command queue
    GL_ASSERT(gl::api::flush());

    for(size_t i = 0; i < render::scene::CSMBuffer::NSplits; ++i)
    {
      m_csm->setActiveSplit(i);
      m_csm->waitActiveBlurSync();
    }
  }

  {
    SOGLB_DEBUGGROUP("geometry-pass");
    m_renderPipeline->bindGeometryFrameBuffer(cameraController.getCamera()->getFarPlane());

    {
      SOGLB_DEBUGGROUP("depth-prefill-pass");

      // collect rooms and sort front-to-back
      std::vector<const world::Room*> renderRooms;
      for(const auto& room : rooms)
      {
        if(room.node->isVisible())
          renderRooms.emplace_back(&room);
      }
      std::sort(renderRooms.begin(),
                renderRooms.end(),
                [](const world::Room* a, const world::Room* b)
                {
                  return a->node->getRenderOrder() > b->node->getRenderOrder();
                });

      for(const auto translucencySelector :
          {render::scene::Translucency::Opaque, render::scene::Translucency::NonOpaque})
      {
        render::scene::RenderContext context{render::material::RenderMode::DepthOnly,
                                             cameraController.getCamera()->getViewProjectionMatrix(),
                                             translucencySelector};
        for(const auto& room : renderRooms)
        {
          SOGLB_DEBUGGROUP(room->node->getName());
          auto state = context.getCurrentState();
          state.setScissorTest(true);
          const auto [xy, size] = room->node->getCombinedScissors();
          state.setScissorRegion(xy, size);
          context.pushState(state);
          room->node->getRenderable()->render(room->node.get(), context);
          context.popState();
        }
        break;
      }
      if constexpr(render::pass::FlushPasses)
        GL_ASSERT(gl::api::finish());
    }

    m_renderer->render();
    for(const auto translucencySelector : {render::scene::Translucency::Opaque, render::scene::Translucency::NonOpaque})
    {
      render::scene::RenderContext context{translucencySelector == render::scene::Translucency::Opaque
                                             ? render::material::RenderMode::FullOpaque
                                             : render::material::RenderMode::FullNonOpaque,
                                           std::nullopt,
                                           translucencySelector};
      for(auto& room : rooms)
      {
        if(!room.node->isVisible())
          continue;

        context.pushState(room.node->getRenderState());
        room.particles.render(context, world);
        context.popState();
      }
    }

    if constexpr(render::pass::FlushPasses)
      GL_ASSERT(gl::api::finish());
  }

  m_renderPipeline->renderPortalFrameBuffer(
    [&cameraController, &waterEntryPortals](const gl::RenderState& fbRenderState)
    {
      gl::RenderState::resetWantedState();

      for(const auto translucencySelector :
          {render::scene::Translucency::Opaque, render::scene::Translucency::NonOpaque})
      {
        render::scene::RenderContext context{render::material::RenderMode::DepthOnly,
                                             cameraController.getCamera()->getViewProjectionMatrix(),
                                             translucencySelector};

        context.pushState(fbRenderState);
        for(const auto& portal : waterEntryPortals)
        {
          portal->mesh->render(nullptr, context);
        }
        context.popState();
      }
      if constexpr(render::pass::FlushPasses)
        GL_ASSERT(gl::api::finish());
    });

  m_renderPipeline->worldCompositionPass(rooms, cameraController.getCurrentRoom()->isWaterRoom);
  m_screenOverlay.reset();
}

namespace
{
constexpr size_t BarColors = 5;
constexpr int BarScale = 3;
constexpr int BarWidth = 100 * BarScale;
constexpr int BarHeight = 5 * BarScale;

gl::SRGBA8 getBarColor(float x, const std::array<gl::SRGBA8, BarColors>& barColors)
{
  x *= BarColors;
  auto n = static_cast<int>(glm::floor(x));
  if(n < 0)
    return barColors[0];
  else if(static_cast<size_t>(n) >= BarColors - 1)
    return barColors[BarColors - 1];

  const auto a = barColors[n];
  const auto b = barColors[n + 1];
  return gl::imix(a, b, static_cast<uint8_t>(glm::mod(x, 1.0f) * 256));
}

void drawBar(ui::Ui& ui,
             const glm::ivec2& xy0,
             const int p,
             const gl::SRGBA8& black,
             const gl::SRGBA8& border1,
             const gl::SRGBA8& border2,
             const std::array<gl::SRGBA8, BarColors>& barColors)
{
  ui.drawBox(xy0 + glm::ivec2{-1, -1}, {BarWidth + 3, BarHeight + 2}, black);
  ui.drawHLine(xy0 + glm::ivec2{-2, BarHeight + 1}, BarWidth + 4, border1);
  ui.drawVLine(xy0 + glm::ivec2{BarWidth + 2, -2}, BarHeight + 3, border1);
  ui.drawHLine(xy0 + glm::ivec2{-2, -2}, BarWidth + 4, border2);
  ui.drawVLine(xy0 + glm::ivec2{-2, -2}, BarHeight + 3, border2);

  if(p > 0)
  {
    for(int i = 0; i < BarHeight; ++i)
      ui.drawHLine(xy0 + glm::ivec2{0, i}, p, getBarColor(static_cast<float>(i) / (BarHeight - 1), barColors));
  }
}
} // namespace

void Presenter::drawBars(ui::Ui& ui,
                         const std::array<gl::SRGBA8, 256>& palette,
                         const ObjectManager& objectManager,
                         bool pulse)
{
  if(objectManager.getLara().isInWater())
  {
    drawBar(ui,
            {ui.getSize().x - BarWidth - 10, 8},
            std::clamp(objectManager.getLara().getAir() * BarWidth / core::LaraAir, 0, BarWidth),
            palette[0],
            palette[17],
            palette[19],
            {
              palette[32],
              palette[41],
              palette[32],
              palette[19],
              palette[21],
            });
  }

  if(objectManager.getLara().getHandStatus() == objects::HandStatus::Combat || objectManager.getLara().isDead())
    m_healthBarTimeout = DefaultHealthBarTimeout;

  {
    const auto laraHealth = std::max(0_hp, objectManager.getLara().m_state.health);
    auto newHealth = m_drawnHealth;
    if(laraHealth < m_drawnHealth)
      newHealth = std::max(m_drawnHealth - HealthChangeDeltaPerFrame, laraHealth);
    else if(laraHealth > m_drawnHealth)
      newHealth = std::min(m_drawnHealth + HealthChangeDeltaPerFrame, laraHealth);

    if(std::exchange(m_drawnHealth, newHealth) != laraHealth)
      m_healthBarTimeout = DefaultHealthBarTimeout;
  }

  m_healthBarTimeout -= 1_frame;

  if(pulse)
  {
    if(m_drawnHealth > HealthPulseMaxHealth && m_healthBarTimeout <= -DefaultHealthBarTimeout)
      return;
  }
  else
  {
    if(m_healthBarTimeout <= -DefaultHealthBarTimeout)
      return;
  }

  uint8_t alpha = 255;
  if(m_healthBarTimeout < 0_frame)
  {
    alpha = gsl::narrow_cast<uint8_t>(std::clamp(255 - std::abs(255 * m_healthBarTimeout / 40_frame), 0, 255));
  }

  if(pulse && m_drawnHealth <= HealthPulseMaxHealth)
  {
    const auto pulseStrength = 1
                               - std::clamp((m_drawnHealth - HealthPulseMinHealth).cast<float>()
                                              / (HealthPulseMaxHealth - HealthPulseMinHealth),
                                            0.0f,
                                            1.0f);
    const auto pulseInterval
      = std::max(1_frame,
                 (HealthPulseDurationFast
                  + (HealthPulseDurationSlow - HealthPulseDurationFast).cast<float>() * (1 - pulseStrength))
                   .cast<int>());
    m_healthPulseTime = (m_healthPulseTime + 1_frame) % pulseInterval;
    float currentPulseEffectStrength = 0;
    if(m_healthPulseTime <= pulseInterval / 2)
      currentPulseEffectStrength = m_healthPulseTime.cast<float>() / (pulseInterval.cast<float>() / 2);
    else
      currentPulseEffectStrength
        = (pulseInterval - m_healthPulseTime).cast<float>() / (pulseInterval.cast<float>() / 2);
    static constexpr uint8_t PulseTargetAlpha = 128;
    alpha = glm::mix(alpha, PulseTargetAlpha, currentPulseEffectStrength);
  }

  static const auto withAlpha = [](gl::SRGBA8 color, uint8_t alpha)
  {
    color.channels.a = alpha;
    return color;
  };

  drawBar(ui,
          {8, 8},
          std::clamp(m_drawnHealth * BarWidth / core::LaraHealth, 0, BarWidth),
          withAlpha(palette[0], alpha),
          withAlpha(palette[17], alpha),
          withAlpha(palette[19], alpha),
          {
            withAlpha(palette[8], alpha),
            withAlpha(palette[11], alpha),
            withAlpha(palette[8], alpha),
            withAlpha(palette[6], alpha),
            withAlpha(palette[24], alpha),
          });
}

namespace
{
std::vector<std::filesystem::path> getIconPaths(const std::filesystem::path& base, const std::vector<int>& sizes)
{
  std::vector<std::filesystem::path> result;
  result.reserve(sizes.size());
  for(const auto size : sizes)
  {
    result.emplace_back(util::ensureFileExists(base / ("logo_" + std::to_string(size) + ".png")));
  }
  return result;
}
} // namespace

Presenter::Presenter(const std::filesystem::path& engineDataPath,
                     const glm::ivec2& resolution,
                     const render::RenderSettings& renderSettings)
    : m_window{std::make_shared<gl::Window>(getIconPaths(engineDataPath, {24, 32, 64, 128, 256, 512}), resolution)}
    , m_soundEngine{std::make_shared<audio::SoundEngine>()}
    , m_renderer{std::make_shared<render::scene::Renderer>(
        gsl::make_shared<render::scene::Camera>(DefaultFov, getRenderViewport(), DefaultNearPlane, DefaultFarPlane))}
    , m_splashImageTexture{gsl::make_shared<gl::TextureHandle<gl::Texture2D<gl::PremultipliedSRGBA8>>>(
        gl::CImgWrapper{util::ensureFileExists(engineDataPath / "splash.png")}.toTexture("splash"),
        gsl::make_unique<gl::Sampler>("splash" + gl::SamplerSuffix))}
    , m_trTTFFont{std::make_unique<gl::Font>(util::ensureFileExists(engineDataPath / "trfont.ttf"))}
    , m_ghostNameFont{std::make_unique<gl::Font>(util::ensureFileExists(engineDataPath / "Roboto-Regular.ttf"))}
    , m_inputHandler{std::make_unique<hid::InputHandler>(m_window, engineDataPath / "gamecontrollerdb.txt")}
    , m_shaderCache{std::make_shared<render::material::ShaderCache>(engineDataPath / "shaders")}
    , m_materialManager{std::make_unique<render::material::MaterialManager>(m_shaderCache, m_renderer)}
    , m_csm{std::make_shared<render::scene::CSM>(renderSettings.getCSMResolution(), *m_materialManager)}
    , m_renderPipeline{std::make_unique<render::RenderPipeline>(
        *m_materialManager, getRenderViewport(), getUiViewport(), getDisplayViewport())}
{
  m_materialManager->setCSM(gsl::not_null{m_csm});
  scaleSplashImage();
  drawLoadingScreen(_("Booting"));
}

Presenter::~Presenter() = default;

void Presenter::scaleSplashImage()
{
  // scale splash image so that its aspect ratio is preserved, but the boundaries match
  const auto viewport = glm::vec2{getDisplayViewport()};
  const auto srcTexture
    = m_splashImageTextureOverride != nullptr ? m_splashImageTextureOverride : m_splashImageTexture.get();
  const auto sourceSize = glm::vec2{srcTexture->getTexture()->size()};
  const float splashScale = std::max(viewport.x / sourceSize.x, viewport.y / sourceSize.y);

  auto scaledSourceSize = sourceSize * splashScale;
  auto sourceOffset = (viewport - scaledSourceSize) / 2.0f;
  auto mesh = render::scene::createScreenQuad(sourceOffset,
                                              scaledSourceSize,
                                              m_materialManager->getBackdrop(false),
                                              render::scene::Translucency::Opaque,
                                              "backdrop");
  if(m_splashImageTextureOverride != nullptr)
    m_splashImageMeshOverride = mesh;
  else
    m_splashImageMesh = mesh;
  mesh->bind("u_input",
             [srcTexture = gsl::not_null{srcTexture}](
               const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
             {
               uniform.set(srcTexture);
             });
}

void Presenter::drawLoadingScreen(const std::string& state)
{
  if(!preFrame())
    return;

  if(m_screenOverlay == nullptr)
    m_screenOverlay = std::make_unique<render::scene::ScreenOverlay>();

  if(m_screenOverlay->getImage()->getSize() != getDisplayViewport())
  {
    m_screenOverlay->init(*m_materialManager, getDisplayViewport());
    scaleSplashImage();
  }

  m_screenOverlay->getImage()->fill(gl::PremultipliedSRGBA8{0, 0, 0, 0});
  m_trTTFFont->drawText(*m_screenOverlay->getImage(),
                        state.c_str(),
                        glm::ivec2{40, m_screenOverlay->getImage()->getSize().y - 100},
                        gl::PremultipliedSRGBA8{255, 255, 255, 255},
                        StatusLineFontSize);

  m_renderer->getCamera()->setViewport(getDisplayViewport());
  getSplashImageMeshOrOverride()->getRenderState().setViewport(getDisplayViewport());

  m_renderPipeline->withBackbuffer(
    [this]()
    {
      {
        render::scene::RenderContext context{
          render::material::RenderMode::FullOpaque, std::nullopt, render::scene::Translucency::Opaque};
        getSplashImageMeshOrOverride()->render(nullptr, context);
      }

      m_screenOverlay->setAlphaMultiplier(0.8f);
      {
        render::scene::RenderContext context{
          render::material::RenderMode::FullNonOpaque, std::nullopt, render::scene::Translucency::NonOpaque};
        m_screenOverlay->render(nullptr, context);
      }
    });

  updateSoundEngine();
  swapBuffers();
}

bool Presenter::preFrame()
{
  m_window->updateWindowSize();
  if(m_window->isMinimized())
    return false;

  m_renderer->getCamera()->setViewport(getRenderViewport());
  m_renderPipeline->resize(*m_materialManager, getRenderViewport(), getUiViewport(), getDisplayViewport());
  if(m_screenOverlay != nullptr)
  {
    if(m_screenOverlay->getImage()->getSize() != getDisplayViewport())
    {
      m_screenOverlay->init(*m_materialManager, getDisplayViewport());
    }

    m_screenOverlay->getImage()->fill({0, 0, 0, 0});
  }

  m_inputHandler->update();

  m_renderer->clear(
    gl::api::ClearBufferMask::ColorBufferBit | gl::api::ClearBufferMask::DepthBufferBit, {0, 0, 0, 0}, 1);
  m_renderPipeline->clearBackbuffer();

  m_renderSettingsChanged = false;

  return true;
}

bool Presenter::shouldClose() const
{
  return m_window->windowShouldClose();
}

void Presenter::setTrFont(std::unique_ptr<ui::TRFont>&& font)
{
  m_trFont = std::move(font);
}

void Presenter::swapBuffers()
{
  m_renderPipeline->renderBackbufferEffects();
  m_window->swapBuffers();
}

void Presenter::clear()
{
  m_renderer->resetRootNode();
  m_renderer->getCamera()->setFieldOfView(DefaultFov);
}

void Presenter::debounceInput()
{
  m_inputHandler->update();
  m_inputHandler->update();
}

void Presenter::apply(const render::RenderSettings& renderSettings, const AudioSettings& audioSettings)
{
  m_renderResolutionDivisor = renderSettings.renderResolutionDivisorActive ? renderSettings.renderResolutionDivisor : 1;
  m_uiScale = renderSettings.uiScaleActive ? renderSettings.uiScaleMultiplier : 1;
  m_renderer->getCamera()->setViewport(getRenderViewport());
  setFullscreen(renderSettings.fullscreen);
  if(m_csm->getResolution() != renderSettings.getCSMResolution())
  {
    m_csm = gsl::make_shared<render::scene::CSM>(renderSettings.getCSMResolution(), *m_materialManager);
    m_materialManager->setCSM(m_csm);
  }
  m_renderPipeline->apply(renderSettings, *m_materialManager);
  m_materialManager->setFiltering(renderSettings.bilinearFiltering,
                                  !renderSettings.anisotropyActive
                                    ? std::nullopt
                                    : std::optional{gsl::narrow<float>(renderSettings.anisotropyLevel)});
  m_soundEngine->setListenerGain(audioSettings.globalVolume);
  m_renderSettingsChanged = true;
}

gl::CImgWrapper Presenter::takeScreenshot() const
{
  const auto vp = getDisplayViewport();

  std::vector<uint8_t> pixels;
  pixels.resize(gsl::narrow<size_t>(vp.x) * gsl::narrow<size_t>(vp.y) * 4u);
  GL_ASSERT(
    gl::api::readPixel(0, 0, vp.x, vp.y, gl::api::PixelFormat::Rgba, gl::api::PixelType::UnsignedByte, pixels.data()));

  gl::CImgWrapper img{pixels.data(), vp.x, vp.y, false};
  img.fromScreenshot();
  return img;
}

void Presenter::renderScreenOverlay()
{
  if(m_screenOverlay == nullptr)
    return;

  SOGLB_DEBUGGROUP("screen-overlay-pass");
  gl::RenderState::resetWantedState();
  m_renderer->getCamera()->setViewport(getDisplayViewport());
  gl::RenderState::getWantedState().setViewport(getDisplayViewport());
  render::scene::RenderContext context{
    render::material::RenderMode::FullNonOpaque, std::nullopt, render::scene::Translucency::NonOpaque};
  m_screenOverlay->render(nullptr, context);
}

void Presenter::renderUi(ui::Ui& ui, float alpha)
{
  m_renderPipeline->renderUiFrameBuffer(
    [this, &ui]()
    {
      m_renderer->getCamera()->setViewport(getUiViewport());
      ui.render();
    });
  m_renderPipeline->renderUiFrameBuffer(alpha);
}

void Presenter::disableScreenOverlay()
{
  m_screenOverlay.reset();
}

void Presenter::updateSoundEngine()
{
  m_soundEngine->update();
}

glm::ivec2 Presenter::getRenderViewport() const
{
  BOOST_ASSERT(m_renderResolutionDivisor > 0);
  return m_window->getViewport() / static_cast<int>(m_renderResolutionDivisor);
}

glm::ivec2 Presenter::getUiViewport() const
{
  BOOST_ASSERT(m_uiScale > 0);
  return m_window->getViewport() / static_cast<int>(m_uiScale);
}

void Presenter::withBackbuffer(const std::function<void()>& doRender)
{
  m_renderPipeline->withBackbuffer(doRender);
}

void Presenter::setSplashImageTextureOverride(const std::filesystem::path& imagePath)
{
  m_splashImageTextureOverride = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::PremultipliedSRGBA8>>>(
    gl::CImgWrapper{util::ensureFileExists(imagePath)}.toTexture("splash-override"),
    gsl::make_unique<gl::Sampler>("splash-override" + gl::SamplerSuffix));
  scaleSplashImage();
}

void Presenter::clearSplashImageTextureOverride()
{
  m_splashImageTextureOverride.reset();
  m_splashImageMeshOverride.reset();
}
} // namespace engine
