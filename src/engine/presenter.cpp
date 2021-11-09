#include "presenter.h"

#include "audio/soundengine.h"
#include "audiosettings.h"
#include "cameracontroller.h"
#include "core/i18n.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "objectmanager.h"
#include "objects/laraobject.h"
#include "objects/object.h"
#include "objects/objectstate.h"
#include "qs/qs.h"
#include "render/pass/config.h"
#include "render/renderpipeline.h"
#include "render/rendersettings.h"
#include "render/scene/camera.h"
#include "render/scene/csm.h"
#include "render/scene/material.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/node.h"
#include "render/scene/renderable.h"
#include "render/scene/rendercontext.h"
#include "render/scene/renderer.h"
#include "render/scene/rendermode.h"
#include "render/scene/screenoverlay.h"
#include "render/scene/shadercache.h"
#include "render/scene/visitor.h"
#include "ui/text.h"
#include "ui/ui.h"
#include "util/helpers.h"
#include "video/videoplayer.h"
#include "world/room.h"

#include <algorithm>
#include <boost/range/adaptor/map.hpp>
#include <cstdint>
#include <cstdlib>
#include <gl/cimgwrapper.h>
#include <gl/debuggroup.h>
#include <gl/fencesync.h>
#include <gl/font.h>
#include <gl/framebuffer.h>
#include <gl/glassert.h>
#include <gl/glfw.h>
#include <gl/image.h>
#include <gl/pixel.h>
#include <gl/program.h>
#include <gl/renderstate.h>
#include <gl/texture2d.h>
#include <gl/texturedepth.h>
#include <gl/texturehandle.h>
#include <gl/window.h>
#include <glm/common.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <gslu.h>
#include <initializer_list>
#include <optional>
#include <set>
#include <utility>

namespace
{
constexpr int StatusLineFontSize = 40;
constexpr int DebugTextFontSize = 12;
} // namespace

namespace engine
{
void Presenter::playVideo(const std::filesystem::path& path)
{
  util::ensureFileExists(path);

  m_soundEngine->setListenerGain(1.0f);

  auto mesh = createScreenQuad(m_materialManager->getFlat(false, true, true), "video");

  video::play(path,
              m_soundEngine->getDevice(),
              [&](const gsl::not_null<std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>>& textureHandle)
              {
                if(update())
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
                  .get(render::scene::RenderMode::Full)
                  ->getUniformBlock("Camera")
                  ->bindCameraBuffer(m_renderer->getCamera());

                if(m_window->isMinimized())
                  return true;

                m_renderer->clear(gl::api::ClearBufferMask::ColorBufferBit, {0, 0, 0, 255}, 1);
                render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};
                mesh->render(nullptr, context);
                updateSoundEngine();
                swapBuffers();
                m_inputHandler->update();
                return !m_window->windowShouldClose() && !m_inputHandler->hasDebouncedAction(hid::Action::Menu);
              });
}

void Presenter::renderWorld(const ObjectManager& objectManager,
                            const std::vector<world::Room>& rooms,
                            const CameraController& cameraController,
                            const std::unordered_set<const world::Portal*>& waterEntryPortals,
                            float waitRatio)
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

      render::scene::RenderContext context{render::scene::RenderMode::CSMDepthOnly,
                                           m_csm->getActiveMatrix(glm::mat4{1.0f})};
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
      render::scene::RenderContext context{render::scene::RenderMode::DepthOnly,
                                           cameraController.getCamera()->getViewProjectionMatrix()};
      for(const auto& room : rooms)
      {
        if(!room.node->isVisible())
          continue;

        SOGLB_DEBUGGROUP(room.node->getName());
        auto state = context.getCurrentState();
        state.setScissorTest(true);
        const auto [xy, size] = room.node->getCombinedScissors();
        state.setScissorRegion(xy, size);
        context.pushState(state);
        room.node->getRenderable()->render(room.node.get(), context);
        context.popState();
      }
      if constexpr(render::pass::FlushPasses)
        GL_ASSERT(gl::api::finish());
    }

    m_renderer->render();

    if constexpr(render::pass::FlushPasses)
      GL_ASSERT(gl::api::finish());
  }

  {
    SOGLB_DEBUGGROUP("portal-depth-pass");
    gl::RenderState::resetWantedState();

    render::scene::RenderContext context{render::scene::RenderMode::DepthOnly,
                                         cameraController.getCamera()->getViewProjectionMatrix()};

    context.pushState(m_renderPipeline->bindPortalFrameBuffer());
    for(const auto& portal : waterEntryPortals)
    {
      portal->mesh->render(nullptr, context);
    }
    if constexpr(render::pass::FlushPasses)
      GL_ASSERT(gl::api::finish());
  }

  m_renderPipeline->worldCompositionPass(cameraController.getCurrentRoom()->isWaterRoom);

  if(m_showDebugInfo)
  {
    if(m_screenOverlay == nullptr)
      m_screenOverlay = std::make_unique<render::scene::ScreenOverlay>();
    if(m_screenOverlay->getImage()->getSize() != getRenderViewport())
      m_screenOverlay->init(*m_materialManager, getRenderViewport());
    m_debugFont->drawText(
      *m_screenOverlay->getImage(),
      std::to_string(waitRatio).c_str(),
      glm::ivec2{m_screenOverlay->getImage()->getSize().x - 80, m_screenOverlay->getImage()->getSize().y - 40},
      gl::SRGBA8{255},
      DebugTextFontSize);

    const auto drawObjectName = [this](const std::shared_ptr<objects::Object>& object, const gl::SRGBA8& color)
    {
      const auto vertex
        = glm::vec3{m_renderer->getCamera()->getViewMatrix() * glm::vec4(object->getNode()->getTranslationWorld(), 1)};

      if(vertex.z > -m_renderer->getCamera()->getNearPlane() || vertex.z < -m_renderer->getCamera()->getFarPlane())
      {
        return;
      }

      glm::vec4 projVertex{vertex, 1};
      projVertex = m_renderer->getCamera()->getProjectionMatrix() * projVertex;
      projVertex /= projVertex.w;

      if(std::abs(projVertex.x) > 1 || std::abs(projVertex.y) > 1)
        return;

      projVertex.x = (projVertex.x / 2 + 0.5f) * getRenderViewport().x;
      projVertex.y = (1 - (projVertex.y / 2 + 0.5f)) * getRenderViewport().y;

      m_debugFont->drawText(*m_screenOverlay->getImage(),
                            object->getNode()->getName().c_str(),
                            glm::ivec2{static_cast<int>(projVertex.x), static_cast<int>(projVertex.y)},
                            color,
                            DebugTextFontSize);
    };

    for(const auto& object : objectManager.getObjects() | boost::adaptors::map_values)
    {
      drawObjectName(object, gl::SRGBA8{255});
    }
    for(const auto& object : objectManager.getDynamicObjects())
    {
      drawObjectName(object, gl::SRGBA8{0, 255, 0, 255});
    }
  }
  else
  {
    m_screenOverlay.reset();
  }
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
  if(n <= 0)
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
  ui.drawBox(xy0 + glm::ivec2{-1, -1}, {BarWidth + 2, BarHeight + 2}, black);
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

void Presenter::drawBars(ui::Ui& ui, const std::array<gl::SRGBA8, 256>& palette, const ObjectManager& objectManager)
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
    m_healthBarTimeout = 40_frame;

  if(std::exchange(m_drawnHealth, objectManager.getLara().m_state.health) != objectManager.getLara().m_state.health)
    m_healthBarTimeout = 40_frame;

  m_healthBarTimeout -= 1_frame;
  if(m_healthBarTimeout <= -40_frame)
    return;

  uint8_t alpha = 255;
  if(m_healthBarTimeout < 0_frame)
  {
    alpha = gsl::narrow_cast<uint8_t>(std::clamp(255 - std::abs(255 * m_healthBarTimeout / 40_frame), 0, 255));
  }

  static const auto withAlpha = [](gl::SRGBA8 color, uint8_t alpha)
  {
    color.channels.a = alpha;
    return color;
  };

  drawBar(ui,
          {8, 8},
          std::clamp(objectManager.getLara().m_state.health * BarWidth / core::LaraHealth, 0, BarWidth),
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

Presenter::Presenter(const std::filesystem::path& engineDataPath, const glm::ivec2& resolution)
    : m_window{std::make_unique<gl::Window>(getIconPaths(engineDataPath, {24, 32, 64, 128, 256, 512}), resolution)}
    , m_soundEngine{std::make_shared<audio::SoundEngine>()}
    , m_renderer{std::make_shared<render::scene::Renderer>(gslu::make_nn_shared<render::scene::Camera>(
        DefaultFov, getRenderViewport(), DefaultNearPlane, DefaultFarPlane))}
    , m_splashImage{gsl::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>(
        gl::CImgWrapper{util::ensureFileExists(engineDataPath / "splash.png")}.toTexture("splash"),
        gslu::make_nn_unique<gl::Sampler>("splash-sampler"))}
    , m_trTTFFont{std::make_unique<gl::Font>(util::ensureFileExists(engineDataPath / "trfont.ttf"))}
    , m_debugFont{std::make_unique<gl::Font>(util::ensureFileExists(engineDataPath / "DroidSansMono.ttf"))}
    , m_inputHandler{std::make_unique<hid::InputHandler>(m_window->getWindow(),
                                                         engineDataPath / "gamecontrollerdb.txt")}
    , m_shaderCache{std::make_shared<render::scene::ShaderCache>(engineDataPath / "shaders")}
    , m_materialManager{std::make_unique<render::scene::MaterialManager>(m_shaderCache, m_renderer)}
    , m_csm{std::make_shared<render::scene::CSM>(1024, *m_materialManager)}
    , m_renderPipeline{
        std::make_unique<render::RenderPipeline>(*m_materialManager, getRenderViewport(), getDisplayViewport())}
{
  m_materialManager->setCSM(gsl::not_null{m_csm});
  scaleSplashImage();
  drawLoadingScreen(_("Booting"));
}

Presenter::~Presenter() = default;

void Presenter::scaleSplashImage()
{
  // scale splash image so that its aspect ratio is preserved, but the boundaries match
  const auto viewport = glm::vec2{getRenderViewport()};
  const auto sourceSize = glm::vec2{m_splashImage->getTexture()->size()};
  const float splashScale = std::max(viewport.x / sourceSize.x, viewport.y / sourceSize.y);

  auto scaledSourceSize = sourceSize * splashScale;
  auto sourceOffset = (viewport - scaledSourceSize) / 2.0f;
  m_splashImageMesh
    = render::scene::createScreenQuad(sourceOffset, scaledSourceSize, m_materialManager->getBackdrop(), "backdrop");
  m_splashImageMesh->bind(
    "u_input",
    [this](const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
    {
      uniform.set(m_splashImage);
    });
}

void Presenter::drawLoadingScreen(const std::string& state)
{
  if(update())
    return;

  if(m_screenOverlay == nullptr)
    m_screenOverlay = std::make_unique<render::scene::ScreenOverlay>();

  if(getRenderViewport() != m_screenOverlay->getImage()->getSize())
  {
    m_renderer->getCamera()->setViewport(getRenderViewport());
    m_screenOverlay->init(*m_materialManager, getRenderViewport());
    scaleSplashImage();
  }

  m_screenOverlay->getImage()->fill(gl::SRGBA8{0, 0, 0, 0});
  m_trTTFFont->drawText(*m_screenOverlay->getImage(),
                        state.c_str(),
                        glm::ivec2{40, m_screenOverlay->getImage()->getSize().y - 100},
                        gl::SRGBA8{255, 255, 255, 255},
                        StatusLineFontSize);

  gl::Framebuffer::unbindAll();

  m_renderer->clear(
    gl::api::ClearBufferMask::ColorBufferBit | gl::api::ClearBufferMask::DepthBufferBit, {0, 0, 0, 0}, 1);
  render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};
  m_splashImageMesh->render(nullptr, context);
  m_screenOverlay->setAlphaMultiplier(0.8f);
  m_screenOverlay->render(nullptr, context);
  updateSoundEngine();
  swapBuffers();
}

bool Presenter::preFrame()
{
  m_window->updateWindowSize();
  if(m_window->isMinimized())
    return false;

  m_renderer->getCamera()->setViewport(getRenderViewport());
  m_renderPipeline->resize(*m_materialManager, getRenderViewport(), getDisplayViewport());
  if(m_screenOverlay != nullptr)
  {
    if(m_screenOverlay->getImage()->getSize() != getRenderViewport())
    {
      m_screenOverlay->init(*m_materialManager, getRenderViewport());
    }

    m_screenOverlay->getImage()->fill({0, 0, 0, 0});
  }

  m_inputHandler->update();

  if(m_inputHandler->hasDebouncedAction(hid::Action::Debug))
  {
    m_showDebugInfo = !m_showDebugInfo;
  }

  m_renderer->clear(
    gl::api::ClearBufferMask::ColorBufferBit | gl::api::ClearBufferMask::DepthBufferBit, {0, 0, 0, 0}, 1);

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
  m_halfRes = renderSettings.halfResRender;
  m_renderer->getCamera()->setViewport(getRenderViewport());
  setFullscreen(renderSettings.fullscreen);
  if(m_csm->getResolution() != renderSettings.getCSMResolution())
  {
    m_csm = gslu::make_nn_shared<render::scene::CSM>(renderSettings.getCSMResolution(), *m_materialManager);
    m_materialManager->setCSM(m_csm);
  }
  m_renderPipeline->apply(renderSettings, *m_materialManager);
  m_materialManager->setFiltering(renderSettings.bilinearFiltering, gsl::narrow<float>(renderSettings.anisotropyLevel));
  m_soundEngine->setListenerGain(audioSettings.globalVolume);
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

  render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};

  SOGLB_DEBUGGROUP("screen-overlay-pass");
  gl::RenderState::resetWantedState();
  m_screenOverlay->render(nullptr, context);
}

void Presenter::renderUi(ui::Ui& ui, float alpha)
{
  m_renderPipeline->bindUiFrameBuffer();
  ui.render();
  m_renderPipeline->renderUiFrameBuffer(alpha);
}

void Presenter::disableScreenOverlay()
{
  m_screenOverlay.reset();
}

bool Presenter::update()
{
  glfwPollEvents();
  m_window->updateWindowSize();
  return m_window->isMinimized();
}

void Presenter::updateSoundEngine()
{
  m_soundEngine->update();
}
} // namespace engine
