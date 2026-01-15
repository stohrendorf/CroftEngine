#pragma once

#include "core/magic.h"
#include "core/units.h"
#include "qs/qs.h"
#include "qs/quantity.h"
#include "render/rendersystem.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <gl/cimgwrapper.h>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h> // IWYU pragma: keep
#include <gl/window.h>
#include <glm/trigonometric.hpp>
#include <glm/vec2.hpp>
#include <gsl-lite/gsl-lite.hpp>
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
struct RenderSettings;
} // namespace render

namespace render::scene
{
class Mesh;
class ScreenOverlay;
} // namespace render::scene

namespace engine::world
{
struct Room;
struct Portal;
class World;
} // namespace engine::world

namespace engine
{
class ObjectManager;
class CameraController;
struct AudioSettings;

class Presenter final
{
public:
  explicit Presenter(const std::filesystem::path& engineDataPath,
                     const glm::ivec2& resolution,
                     const render::RenderSettings& renderSettings,
                     bool borderlessFullscreen,
                     std::function<float()> interTickFactorProvider);
  ~Presenter();

  void playVideo(const std::filesystem::path& path);

  /**
   * @brief Renders the world to the backbuffer.
   */
  void renderWorldGeometryFramebuffers(const std::vector<world::Room>& visibleRooms,
                                       const CameraController& cameraController,
                                       const std::unordered_set<const world::Portal*>& waterSurfacePortals,
                                       const world::World& world);

  [[nodiscard]] const auto& getSoundEngine() const noexcept
  {
    return m_soundEngine;
  }

  [[nodiscard]] auto& getRenderSystem() noexcept
  {
    return *m_renderSystem;
  }

  [[nodiscard]] const auto& getRenderSystem() const noexcept
  {
    return *m_renderSystem;
  }

  void initHealthBarTimeout() noexcept
  {
    m_healthBarTimeout = core::DefaultHealthBarTimeout;
  }

  [[nodiscard]] const hid::InputHandler& getInputHandler() const
  {
    return *m_inputHandler;
  }

  [[nodiscard]] hid::InputHandler& getInputHandler()
  {
    return *m_inputHandler;
  }

  void constructBars(ui::Ui& ui,
                     const std::array<gl::SRGBA8, 256>& palette,
                     const ObjectManager& objectManager,
                     bool pulse);

  [[nodiscard]] const ui::TRFont& getTrFont() const
  {
    gsl_Expects(m_trFont != nullptr);
    return *m_trFont;
  }

  void renderScreenOverlayToBackbuffer();
  /**
   * @brief Renders the UI to the backbuffer.
   */
  void renderUiToBackbuffer(ui::Ui& ui, float alpha);

  void apply(const render::RenderSettings& renderSettings, const AudioSettings& audioSettings);

  void drawLoadingScreen(const std::string& state);
  bool beginFrame();
  [[nodiscard]] bool shouldClose() const;

  void setTrFont(std::unique_ptr<ui::TRFont>&& font) noexcept;

  void swapBuffers();

  void clear();

  void debounceInput();

  void setFullscreen(const bool value)
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

  void disableScreenOverlay() noexcept;

  void updateSoundEngine();

  void inBackbuffer(const std::function<void()>& doRender);

  [[nodiscard]] bool renderSettingsChanged() const noexcept
  {
    return m_renderSettingsChanged;
  }

  void setSplashImageTextureOverride(const std::filesystem::path& imagePath);
  void clearSplashImageTextureOverride() noexcept;

  [[nodiscard]] auto& getGhostNameFont() const
  {
    return *m_ghostNameFont;
  }

private:
  gslu::nn_shared<gl::Window> m_window;
  uint8_t m_renderResolutionDivisor = 1;
  uint8_t m_uiScale = 1;

  gslu::nn_shared<audio::SoundEngine> m_soundEngine;
  gslu::nn_shared<render::scene::SceneGraph> m_sceneGraph;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::PremultipliedSRGBA8>>> m_splashImageTexture;
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::PremultipliedSRGBA8>>> m_splashImageTextureOverride;
  std::shared_ptr<render::scene::Mesh> m_splashImageMesh;
  std::shared_ptr<render::scene::Mesh> m_splashImageMeshOverride;
  gslu::nn_unique<gl::Font> m_trTTFFont;
  gslu::nn_unique<gl::Font> m_ghostNameFont;
  core::Health m_drawnHealth = core::LaraHealth;
  core::Frame m_healthPulseTime = 0_frame;
  core::Frame m_healthBarTimeout = -core::DefaultHealthBarTimeout;
  gslu::nn_unique<hid::InputHandler> m_inputHandler;
  std::unique_ptr<ui::TRFont> m_trFont;

  gslu::nn_unique<render::RenderSystem> m_renderSystem;
  std::unique_ptr<render::scene::ScreenOverlay> m_screenOverlay;

  bool m_renderSettingsChanged = false;

  void scaleSplashImage();

  [[nodiscard]] const auto& getSplashImageMeshOrOverride() const noexcept
  {
    if(m_splashImageMeshOverride != nullptr)
      return m_splashImageMeshOverride;
    return m_splashImageMesh;
  }

  void renderGeometry(const world::World& world, const std::vector<world::Room>& rooms);

  void renderCsmBuffers(const std::vector<world::Room>& visibleRooms);
};
} // namespace engine