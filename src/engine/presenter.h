#pragma once

#include "core/magic.h"
#include "core/units.h"
#include "qs/quantity.h"

#include <array>
#include <boost/assert.hpp>
#include <filesystem>
#include <gl/cimgwrapper.h>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h> // IWYU pragma: keep
#include <gl/window.h>
#include <glm/trigonometric.hpp>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

// IWYU pragma: no_forward_declare gl::Font
// IWYU pragma: no_forward_declare gl::Texture2D
// IWYU pragma: no_forward_declare gl::TextureHandle

namespace audio
{
class SoundEngine;
}

namespace ui
{
class TRFont;
class Ui;
} // namespace ui

namespace hid
{
class InputHandler;
}

namespace render
{
class RenderPipeline;
struct RenderSettings;
} // namespace render

namespace render::scene
{
class CSM;
class MaterialManager;
class Mesh;
class Renderer;
class ScreenOverlay;
class ShaderCache;
} // namespace render::scene

namespace engine::world
{
struct Room;
struct Portal;
} // namespace engine::world

namespace engine
{
class ObjectManager;
class CameraController;
struct AudioSettings;

class Presenter final
{
public:
  static const constexpr float DefaultNearPlane = 20.0f;
  static const constexpr float DefaultFarPlane = 20480.0f;
  static const constexpr float DefaultFov = glm::radians(60.0f);

  explicit Presenter(const std::filesystem::path& engineDataPath, const glm::ivec2& resolution);
  ~Presenter();

  void playVideo(const std::filesystem::path& path);

  void renderWorld(const ObjectManager& objectManager,
                   const std::vector<world::Room>& rooms,
                   const CameraController& cameraController,
                   const std::unordered_set<const world::Portal*>& waterEntryPortals,
                   float waitRatio);

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

  void drawBars(ui::Ui& ui, const std::array<gl::SRGBA8, 256>& palette, const ObjectManager& objectManager);

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

  void renderScreenOverlay();
  void renderUi(ui::Ui& ui, float alpha);

  void apply(const render::RenderSettings& renderSettings, const AudioSettings& audioSettings);

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

  [[nodiscard]] gl::CImgWrapper takeScreenshot() const;

  void disableScreenOverlay();

  bool update();

  void updateSoundEngine();

private:
  const std::unique_ptr<gl::Window> m_window;

  std::shared_ptr<audio::SoundEngine> m_soundEngine;
  const std::shared_ptr<render::scene::Renderer> m_renderer;
  const std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>> m_splashImage;
  std::shared_ptr<render::scene::Mesh> m_splashImageMesh;
  const std::unique_ptr<gl::Font> m_trTTFFont;
  const std::unique_ptr<gl::Font> m_debugFont;
  core::Health m_drawnHealth = core::LaraHealth;
  core::Frame m_healthBarTimeout = -40_frame;
  const std::unique_ptr<hid::InputHandler> m_inputHandler;
  std::unique_ptr<ui::TRFont> m_trFont;

  const std::shared_ptr<render::scene::ShaderCache> m_shaderCache{};
  const std::unique_ptr<render::scene::MaterialManager> m_materialManager;
  std::shared_ptr<render::scene::CSM> m_csm{};

  const std::unique_ptr<render::RenderPipeline> m_renderPipeline;
  std::unique_ptr<render::scene::ScreenOverlay> m_screenOverlay;

  bool m_showDebugInfo = false;

  void scaleSplashImage();
};
} // namespace engine
