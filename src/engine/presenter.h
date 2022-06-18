#pragma once

#include "core/magic.h"
#include "core/units.h"
#include "qs/quantity.h"

#include <array>
#include <filesystem>
#include <gl/cimgwrapper.h>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h> // IWYU pragma: keep
#include <gl/window.h>
#include <glm/trigonometric.hpp>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

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
  static const constexpr core::Frame DefaultHealthBarTimeout = core::FrameRate * 1_sec * 4 / 3;

  explicit Presenter(const std::filesystem::path& engineDataPath, const glm::ivec2& resolution);
  ~Presenter();

  void playVideo(const std::filesystem::path& path);

  void renderWorld(const std::vector<world::Room>& rooms,
                   const CameraController& cameraController,
                   const std::unordered_set<const world::Portal*>& waterEntryPortals);

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

  void initHealthBarTimeout()
  {
    m_healthBarTimeout = DefaultHealthBarTimeout;
  }

  [[nodiscard]] const hid::InputHandler& getInputHandler() const
  {
    return *m_inputHandler;
  }

  [[nodiscard]] hid::InputHandler& getInputHandler()
  {
    return *m_inputHandler;
  }

  void drawBars(ui::Ui& ui, const std::array<gl::SRGBA8, 256>& palette, const ObjectManager& objectManager, bool pulse);

  [[nodiscard]] const ui::TRFont& getTrFont() const
  {
    Expects(m_trFont != nullptr);
    return *m_trFont;
  }

  [[nodiscard]] const auto& getRenderer() const
  {
    return *m_renderer;
  }

  [[nodiscard]] auto& getRenderer()
  {
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

  [[nodiscard]] const auto& getDisplayViewport() const
  {
    return m_window->getViewport();
  }

  [[nodiscard]] glm::ivec2 getRenderViewport() const;

  [[nodiscard]] glm::ivec2 getUiViewport() const;

  [[nodiscard]] gl::CImgWrapper takeScreenshot() const;

  void disableScreenOverlay();

  void updateSoundEngine();

private:
  const std::unique_ptr<gl::Window> m_window;
  uint8_t m_renderResolutionDivisor = 1;
  uint8_t m_uiScale = 1;

  std::shared_ptr<audio::SoundEngine> m_soundEngine;
  const gslu::nn_shared<render::scene::Renderer> m_renderer;
  const gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::PremultipliedSRGBA8>>> m_splashImage;
  std::shared_ptr<render::scene::Mesh> m_splashImageMesh;
  const gslu::nn_unique<gl::Font> m_trTTFFont;
  const gslu::nn_unique<gl::Font> m_debugFont;
  core::Health m_drawnHealth = core::LaraHealth;
  core::Frame m_healthPulseTime = 0_frame;
  core::Frame m_healthBarTimeout = -DefaultHealthBarTimeout;
  const gslu::nn_unique<hid::InputHandler> m_inputHandler;
  std::unique_ptr<ui::TRFont> m_trFont;

  const gslu::nn_shared<render::scene::ShaderCache> m_shaderCache;
  const gslu::nn_unique<render::scene::MaterialManager> m_materialManager;
  gslu::nn_shared<render::scene::CSM> m_csm;

  const gslu::nn_unique<render::RenderPipeline> m_renderPipeline;
  std::unique_ptr<render::scene::ScreenOverlay> m_screenOverlay;

  void scaleSplashImage();
};
} // namespace engine
