#pragma once

#include <filesystem>
#include <functional>
#include <glm/vec2.hpp>
#include <gslu.h>
#include <memory>

namespace render
{
class RenderPipeline;
struct RenderSettings;
} // namespace render

namespace render::scene
{
class Camera;
class CSM;
class SceneGraph;
} // namespace render::scene

namespace render::material
{
class MaterialManager;
class ShaderCache;
} // namespace render::material

namespace render
{
/**
 * @brief Complete rendering subsystem.
 *
 * This class consolidates all rendering infrastructure including the scene graph,
 * rendering assets (shaders, materials), shadow maps (CSM), and the render pipeline.
 * It provides a clean interface for high-level rendering operations.
 */
class RenderSystem final
{
public:
  RenderSystem(const std::filesystem::path& engineDataPath,
               const glm::ivec2& renderViewport,
               const glm::ivec2& uiViewport,
               const glm::ivec2& displayViewport,
               const RenderSettings& renderSettings,
               std::function<float()> interTickFactorProvider);

  ~RenderSystem();

  [[nodiscard]] auto& getSceneGraph() noexcept
  {
    return *m_sceneGraph;
  }

  [[nodiscard]] const auto& getSceneGraph() const noexcept
  {
    return *m_sceneGraph;
  }

  [[nodiscard]] auto& getRenderPipeline() noexcept
  {
    return *m_renderPipeline;
  }

  [[nodiscard]] const auto& getRenderPipeline() const noexcept
  {
    return *m_renderPipeline;
  }

  [[nodiscard]] auto& getCSM() noexcept
  {
    return *m_csm;
  }

  [[nodiscard]] const auto& getCSM() const noexcept
  {
    return *m_csm;
  }

  [[nodiscard]] auto& getMaterialManager() noexcept
  {
    return *m_materialManager;
  }

  [[nodiscard]] const auto& getMaterialManager() const noexcept
  {
    return *m_materialManager;
  }

  [[nodiscard]] const auto& getCamera() const noexcept
  {
    return m_camera;
  }

  void apply(const RenderSettings& renderSettings,
             const glm::ivec2& renderViewport,
             const glm::ivec2& uiViewport,
             const glm::ivec2& displayViewport);

private:
  gslu::nn_shared<scene::Camera> m_camera;
  gslu::nn_shared<scene::SceneGraph> m_sceneGraph;
  gslu::nn_shared<material::ShaderCache> m_shaderCache;
  gslu::nn_unique<material::MaterialManager> m_materialManager;
  gslu::nn_shared<scene::CSM> m_csm;
  std::unique_ptr<RenderPipeline> m_renderPipeline;
};
} // namespace render
