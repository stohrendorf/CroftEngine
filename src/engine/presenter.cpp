#include "presenter.h"

#include "audio/soundengine.h"
#include "audioengine.h"
#include "core/i18n.h"
#include "engine.h"
#include "loader/file/level/level.h"
#include "objectmanager.h"
#include "objects/laraobject.h"
#include "render/pass/config.h"
#include "render/renderpipeline.h"
#include "render/scene/camera.h"
#include "render/scene/csm.h"
#include "render/scene/materialmanager.h"
#include "render/scene/node.h"
#include "render/scene/rendercontext.h"
#include "render/scene/renderer.h"
#include "render/scene/screenoverlay.h"
#include "render/scene/shadercache.h"
#include "render/textureanimator.h"
#include "ui/text.h"
#include "ui/ui.h"
#include "video/player.h"
#include "world/room.h"

#include <boost/range/adaptors.hpp>
#include <gl/debuggroup.h>
#include <gl/font.h>
#include <gl/texture2d.h>

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
  mesh->getRenderState().setBlend(false);

  video::play(
    path,
    m_soundEngine->getDevice(),
    [&](const std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>& textureHandle)
    {
      if(update())
        return true;

      m_renderer->getCamera()->setScreenSize(m_window->getViewport());
      mesh->bind(
        "u_input",
        [&textureHandle](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
        { uniform.set(textureHandle); });
      mesh->bind("u_aspectRatio",
                 [this](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                 {
                   const auto vp = m_window->getViewport();
                   uniform.set(static_cast<float>(vp.x) / static_cast<float>(vp.y));
                 });

      if(m_window->isMinimized())
        return true;

      m_renderer->clear(gl::api::ClearBufferMask::ColorBufferBit, {0, 0, 0, 255}, 1);
      render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};
      mesh->render(context);
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
                            float delayRatio)
{
  m_renderPipeline->updateCamera(m_renderer->getCamera());

  {
    SOGLB_DEBUGGROUP("csm-pass");
    gl::RenderState::resetWantedState();
    gl::RenderState::getWantedState().setDepthClamp(true);
    m_csm->updateCamera(*m_renderer->getCamera());
    m_csm->applyViewport();

    for(size_t i = 0; i < render::scene::CSMBuffer::NSplits; ++i)
    {
      SOGLB_DEBUGGROUP("csm-pass/" + std::to_string(i));

      m_csm->setActiveSplit(i);
      m_csm->getActiveFramebuffer()->bindWithAttachments();
      m_renderer->clear(gl::api::ClearBufferMask::DepthBufferBit, {0, 0, 0, 0}, 1);

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
    }

    for(size_t i = 0; i < render::scene::CSMBuffer::NSplits; ++i)
    {
      SOGLB_DEBUGGROUP("csm-pass-square/" + std::to_string(i));
      m_csm->setActiveSplit(i);
      m_csm->renderSquare();
    }
    for(size_t i = 0; i < render::scene::CSMBuffer::NSplits; ++i)
    {
      SOGLB_DEBUGGROUP("csm-pass-blur/" + std::to_string(i));
      m_csm->setActiveSplit(i);
      m_csm->renderBlur();
    }
  }

  {
    SOGLB_DEBUGGROUP("geometry-pass");
    m_renderPipeline->bindGeometryFrameBuffer(m_window->getViewport());
    m_renderer->clear(
      gl::api::ClearBufferMask::ColorBufferBit | gl::api::ClearBufferMask::DepthBufferBit, {0, 0, 0, 0}, 1);

    {
      SOGLB_DEBUGGROUP("depth-prefill-pass");
      gl::RenderState::resetWantedState();
      render::scene::RenderContext context{render::scene::RenderMode::DepthOnly,
                                           cameraController.getCamera()->getViewProjectionMatrix()};
      for(const auto& room : rooms)
      {
        if(!room.node->isVisible())
          continue;

        SOGLB_DEBUGGROUP(room.node->getName());
        context.setCurrentNode(room.node.get());
        auto state = context.getCurrentState();
        state.setScissorTest(true);
        const auto [xy, size] = room.node->getCombinedScissors();
        state.setScissorRegion(xy, size);
        context.pushState(state);
        room.node->getRenderable()->render(context);
        context.popState();
      }
      if constexpr(render::pass::FlushPasses)
        GL_ASSERT(gl::api::finish());
    }

    gl::RenderState::resetWantedState();
    m_renderer->render();

    if constexpr(render::pass::FlushPasses)
      GL_ASSERT(gl::api::finish());
  }

  {
    SOGLB_DEBUGGROUP("portal-depth-pass");
    gl::RenderState::resetWantedState();

    render::scene::RenderContext context{render::scene::RenderMode::DepthOnly,
                                         cameraController.getCamera()->getViewProjectionMatrix()};

    m_renderPipeline->bindPortalFrameBuffer();
    for(const auto& portal : waterEntryPortals)
    {
      portal->mesh->render(context);
    }
    if constexpr(render::pass::FlushPasses)
      GL_ASSERT(gl::api::finish());
  }

  m_renderPipeline->compositionPass(cameraController.getCurrentRoom()->isWaterRoom);

  if(m_showDebugInfo)
  {
    if(m_screenOverlay == nullptr)
      m_screenOverlay = std::make_unique<render::scene::ScreenOverlay>(*m_materialManager, m_window->getViewport());
    m_debugFont->drawText(
      *m_screenOverlay->getImage(),
      std::to_string(m_renderer->getFrameRate()).c_str(),
      glm::ivec2{m_screenOverlay->getImage()->getSize().x - 80, m_screenOverlay->getImage()->getSize().y - 20},
      gl::SRGBA8{255},
      DebugTextFontSize);
    m_debugFont->drawText(
      *m_screenOverlay->getImage(),
      std::to_string(delayRatio).c_str(),
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

      projVertex.x = (projVertex.x / 2 + 0.5f) * m_window->getViewport().x;
      projVertex.y = (1 - (projVertex.y / 2 + 0.5f)) * m_window->getViewport().y;

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
};
} // namespace

void Presenter::drawBars(ui::Ui& ui, const std::array<gl::SRGBA8, 256>& palette, const ObjectManager& objectManager)
{
  if(objectManager.getLara().isInWater())
  {
    drawBar(ui,
            {m_window->getViewport().x - BarWidth - 10, 8},
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

Presenter::Presenter(const std::filesystem::path& rootPath, const glm::ivec2& resolution)
    : m_window{std::make_unique<gl::Window>(resolution)}
    , m_soundEngine{std::make_shared<audio::SoundEngine>()}
    , m_renderer{std::make_shared<render::scene::Renderer>(std::make_shared<render::scene::Camera>(
        DefaultFov, m_window->getViewport(), DefaultNearPlane, DefaultFarPlane))}
    , m_splashImage{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>(
        gl::CImgWrapper{util::ensureFileExists(rootPath / "share" / "splash.png")}.toTexture())}
    , m_trTTFFont{std::make_unique<gl::Font>(util::ensureFileExists(rootPath / "share" / "trfont.ttf"))}
    , m_debugFont{std::make_unique<gl::Font>(util::ensureFileExists(rootPath / "share" / "DroidSansMono.ttf"))}
    , m_inputHandler{std::make_unique<hid::InputHandler>(m_window->getWindow(),
                                                         rootPath / "share" / "gamecontrollerdb.txt")}
    , m_shaderCache{std::make_shared<render::scene::ShaderCache>(rootPath / "shaders")}
    , m_materialManager{std::make_unique<render::scene::MaterialManager>(m_shaderCache, m_renderer)}
    , m_csm{std::make_shared<render::scene::CSM>(1024, *m_materialManager)}
    , m_renderPipeline{std::make_unique<render::RenderPipeline>(*m_materialManager, m_window->getViewport())}
{
  m_materialManager->setCSM(m_csm);
  scaleSplashImage();
  drawLoadingScreen(_("Booting"));
}

Presenter::~Presenter() = default;

void Presenter::scaleSplashImage()
{
  // scale splash image so that its aspect ratio is preserved, but the boundaries match
  const auto targetSize = glm::vec2{m_window->getViewport()};
  const auto sourceSize = glm::vec2{m_splashImage->getTexture()->size()};
  const float splashScale = std::max(targetSize.x / sourceSize.x, targetSize.y / sourceSize.y);

  auto scaledSourceSize = sourceSize * splashScale;
  auto sourceOffset = (targetSize - scaledSourceSize) / 2.0f;
  m_splashImageMesh
    = render::scene::createScreenQuad(sourceOffset, scaledSourceSize, m_materialManager->getBackdrop(), "backdrop");
  m_splashImageMesh->bind(
    "u_input",
    [this](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
    { uniform.set(m_splashImage); });
  m_splashImageMesh->bind(
    "u_screenSize",
    [targetSize](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
    { uniform.set(targetSize); });
}

void Presenter::drawLoadingScreen(const std::string& state)
{
  if(update())
    return;

  if(m_screenOverlay == nullptr)
    m_screenOverlay = std::make_unique<render::scene::ScreenOverlay>(*m_materialManager, m_window->getViewport());

  if(m_window->getViewport() != m_screenOverlay->getImage()->getSize())
  {
    m_renderer->getCamera()->setScreenSize(m_window->getViewport());
    m_screenOverlay->init(*m_materialManager, m_window->getViewport());
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
  m_splashImageMesh->render(context);
  m_screenOverlay->setAlphaMultiplier(0.8f);
  m_screenOverlay->render(context);
  updateSoundEngine();
  swapBuffers();
}

bool Presenter::preFrame()
{
  m_window->updateWindowSize();
  if(m_window->isMinimized())
    return false;

  m_renderer->getCamera()->setScreenSize(m_window->getViewport());
  m_renderPipeline->resize(*m_materialManager, m_window->getViewport());
  if(m_screenOverlay != nullptr)
  {
    if(m_screenOverlay->getImage()->getSize() != m_window->getViewport())
    {
      m_screenOverlay->init(*m_materialManager, m_window->getViewport());
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
  setFullscreen(renderSettings.fullscreen);
  if(m_csm->getResolution() != renderSettings.getCSMResolution())
  {
    m_csm = std::make_shared<render::scene::CSM>(renderSettings.getCSMResolution(), *m_materialManager);
    m_materialManager->setCSM(m_csm);
  }
  m_renderPipeline->apply(renderSettings, *m_materialManager);
  m_materialManager->setFiltering(renderSettings.bilinearFiltering, gsl::narrow<float>(renderSettings.anisotropyLevel));
  m_soundEngine->setListenerGain(audioSettings.globalVolume);
}

gl::CImgWrapper Presenter::takeScreenshot() const
{
  const auto vp = m_window->getViewport();

  std::vector<uint8_t> pixels;
  pixels.resize(vp.x * vp.y * 4);
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
  m_screenOverlay->render(context);
}

void Presenter::renderUi(ui::Ui& ui, float alpha)
{
  m_renderPipeline->bindUiFrameBuffer();
  ui.render(getViewport());
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
