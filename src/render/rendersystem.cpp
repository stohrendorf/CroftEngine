#include "rendersystem.h"

#include "core/magic.h"
#include "material/materialmanager.h"
#include "material/shadercache.h"
#include "renderpipeline.h"
#include "rendersettings.h"
#include "scene/camera.h"
#include "scene/csm.h"
#include "scene/scenegraph.h"

#include <gsl-lite/gsl-lite.hpp>
#include <utility>

namespace render
{
RenderSystem::RenderSystem(const std::filesystem::path& engineDataPath,
                           const glm::ivec2& renderViewport,
                           const glm::ivec2& uiViewport,
                           const glm::ivec2& displayViewport,
                           const RenderSettings& renderSettings,
                           std::function<float()> interTickFactorProvider)
    : m_camera{gsl_lite::make_shared<scene::Camera>(
        core::DefaultFov, renderViewport, core::DefaultNearPlane, core::DefaultFarPlane)}
    , m_sceneGraph{gsl_lite::make_shared<scene::SceneGraph>(m_camera)}
    , m_shaderCache{gsl_lite::make_shared<material::ShaderCache>(engineDataPath / "shaders")}
    , m_materialManager{gsl_lite::make_unique<material::MaterialManager>(m_shaderCache,
                                                                         m_camera,
                                                                         std::move(interTickFactorProvider),
                                                                         [this]()
                                                                         {
                                                                           return m_sceneGraph->getGameTime();
                                                                         })}
    , m_csm{gsl_lite::make_shared<scene::CSM>(renderSettings.getCSMResolution(), *m_materialManager)}
{
  m_materialManager->setCSM(m_csm);
  m_renderPipeline = std::make_unique<RenderPipeline>(*m_materialManager, renderViewport, uiViewport, displayViewport);
}

RenderSystem::~RenderSystem() = default;

void RenderSystem::apply(const RenderSettings& renderSettings,
                         const glm::ivec2& renderViewport,
                         const glm::ivec2& uiViewport,
                         const glm::ivec2& displayViewport)
{
  m_camera->setViewport(renderViewport);

  if(m_csm->getResolution() != renderSettings.getCSMResolution())
  {
    m_csm = gsl_lite::make_shared<scene::CSM>(renderSettings.getCSMResolution(), *m_materialManager);
    m_materialManager->setCSM(gsl_lite::not_null{m_csm});
  }

  m_renderPipeline->apply(renderSettings, *m_materialManager);
  m_materialManager->setFiltering(renderSettings.bilinearFiltering,
                                  !renderSettings.anisotropyActive
                                    ? std::nullopt
                                    : std::optional{static_cast<float>(renderSettings.anisotropyLevel)});

  m_renderPipeline->resize(*m_materialManager, renderViewport, uiViewport, displayViewport);
}
} // namespace render
