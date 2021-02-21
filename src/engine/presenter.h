#pragma once

#include "core/magic.h"
#include "hid/inputhandler.h"

#include <boost/assert.hpp>
#include <filesystem>
#include <gl/cimgwrapper.h>
#include <gl/window.h>
#include <unordered_set>

namespace loader::file
{
struct Palette;
struct Room;
struct TextureTile;
struct Portal;

namespace level
{
class Level;
}
} // namespace loader::file

namespace audio
{
class SoundEngine;
}

namespace ui
{
class TRFont;
class Ui;
} // namespace ui

namespace render
{
class RenderPipeline;
struct RenderSettings;

namespace scene
{
class ScreenOverlay;
class CSM;
class MaterialManager;
class ShaderManager;
class Renderer;
} // namespace scene
} // namespace render

namespace gl
{
class Font;
template<typename>
class Texture2DArray;
template<typename>
class Image;
} // namespace gl

namespace engine
{
class AudioEngine;
class Engine;
class ObjectManager;
class CameraController;

class Presenter final
{
public:
  static const constexpr float DefaultNearPlane = 20.0f;
  static const constexpr float DefaultFarPlane = 20480.0f;
  static const constexpr float DefaultFov = glm::radians(80.0f);

  explicit Presenter(const std::filesystem::path& rootPath, bool fullscreen, const glm::ivec2& resolution);
  ~Presenter();

  void playVideo(const std::filesystem::path& path);

  void renderWorld(ui::Ui& ui,
                   const ObjectManager& objectManager,
                   const std::vector<loader::file::Room>& rooms,
                   const CameraController& cameraController,
                   const std::unordered_set<const loader::file::Portal*>& waterEntryPortals);

  [[nodiscard]] const auto& getSoundEngine() const
  {
    return m_soundEngine;
  }

  const auto& getMaterialManager()
  {
    return m_materialManager;
  }

  [[nodiscard]] const auto& getMaterialManager() const
  {
    return m_materialManager;
  }

  void setHealthBarTimeout(const core::Frame& f)
  {
    m_healthBarTimeout = f;
  }

  [[nodiscard]] const hid::InputHandler& getInputHandler() const
  {
    return *m_inputHandler;
  }

  [[nodiscard]] hid::InputHandler& getInputHandler()
  {
    return *m_inputHandler;
  }

  void drawBars(ui::Ui& ui, const loader::file::Palette& palette, const ObjectManager& objectManager);

  [[nodiscard]] const ui::TRFont& getTrFont() const
  {
    Expects(m_trFont != nullptr);
    return *m_trFont;
  }

  [[nodiscard]] const auto& getRenderer() const
  {
    BOOST_ASSERT(m_renderer != nullptr);
    return *m_renderer;
  }

  [[nodiscard]] auto& getRenderer()
  {
    BOOST_ASSERT(m_renderer != nullptr);
    return *m_renderer;
  }

  [[nodiscard]] const auto& getScreenOverlay() const
  {
    BOOST_ASSERT(m_screenOverlay != nullptr);
    return *m_screenOverlay;
  }

  [[nodiscard]] auto& getScreenOverlay()
  {
    BOOST_ASSERT(m_screenOverlay != nullptr);
    return *m_screenOverlay;
  }

  [[nodiscard]] const auto& getRenderPipeline() const
  {
    BOOST_ASSERT(m_renderPipeline != nullptr);
    return m_renderPipeline;
  }

  void apply(const render::RenderSettings& renderSettings);

  void drawLoadingScreen(const std::string& state);
  bool preFrame();
  [[nodiscard]] bool shouldClose() const;

  void setTrFont(std::unique_ptr<ui::TRFont>&& font);

  void swapBuffers();

  void clear();

  void debounceInput();

  void setFullscreen(bool value)
  {
    m_window->setFullscreen(value);
  }

  [[nodiscard]] const auto& getViewport() const
  {
    return m_window->getViewport();
  }

  gl::CImgWrapper takeScreenshot() const;

private:
  static constexpr int32_t CSMResolution = 2048;

  const std::unique_ptr<gl::Window> m_window;

  std::shared_ptr<audio::SoundEngine> m_soundEngine;
  const std::shared_ptr<render::scene::Renderer> m_renderer;
  const gl::CImgWrapper m_splashImage;
  gl::CImgWrapper m_splashImageScaled;
  const std::unique_ptr<gl::Font> m_abibasFont;
  const std::unique_ptr<gl::Font> m_debugFont;
  core::Health m_drawnHealth = core::LaraHealth;
  core::Frame m_healthBarTimeout = -40_frame;
  const std::unique_ptr<hid::InputHandler> m_inputHandler;
  std::unique_ptr<ui::TRFont> m_trFont;

  const std::shared_ptr<render::scene::ShaderManager> m_shaderManager{};
  const std::shared_ptr<render::scene::CSM> m_csm{};
  const std::unique_ptr<render::scene::MaterialManager> m_materialManager;

  const std::unique_ptr<render::RenderPipeline> m_renderPipeline;
  const std::unique_ptr<render::scene::ScreenOverlay> m_screenOverlay;

  bool m_showDebugInfo = false;

  void scaleSplashImage();
};
} // namespace engine
