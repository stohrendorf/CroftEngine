#include "controllerbuttons.h"

#include "engine/engineconfig.h"
#include "render/scene/sprite.h"
#include "render/textureatlas.h"
#include "serialization/map.h"
#include "serialization/serialization.h"
#include "serialization/variant.h"
#include "serialization/vector.h"
#include "serialization/yamldocument.h"
#include "ui/core.h"
#include "util/helpers.h"

#include <boost/log/trivial.hpp>
#include <gl/cimgwrapper.h>

namespace engine
{
ControllerLayouts loadControllerButtonIcons(render::MultiTextureAtlas& atlases,
                                            const std::filesystem::path& configFile,
                                            const std::shared_ptr<render::scene::Material>& material)
{
  struct Layout
  {
    std::string name{};
    std::map<std::variant<NamedGlfwGamepadButton, NamedGlfwAxis>, std::string> icons;

    [[nodiscard]] static Layout create(const serialization::Serializer<int>& ser)
    {
      Layout result{};
      ser(S_NV("name", result.name), S_NV("icons", result.icons));
      return result;
    }
  };
  std::vector<Layout> layouts;
  {
    int context{};
    serialization::YAMLDocument<true> doc{configFile};
    doc.load("layouts", context, layouts);
  }

  ControllerLayouts controllerLayouts;
  for(const auto& layout : layouts)
  {
    BOOST_LOG_TRIVIAL(debug) << "Loading controller buttons: " << layout.name;
    ControllerLayout currentLayout;
    for(const auto& [buttonOrAxis, icon] : layout.icons)
    {
      gl::CImgWrapper src{util::ensureFileExists(configFile.parent_path() / icon)};
      const auto newHeight = ui::FontHeight * 2;
      const auto newWidth = src.width() * newHeight / src.height();
      src.resize({newWidth, newHeight});

      const auto atlasLoc = atlases.put(src);
      const glm::vec2 uvLoc
        = (glm::vec2{atlasLoc.second} + glm::vec2{0.5f, 0.5f}) / gsl::narrow_cast<float>(atlases.getSize());
      const glm::vec2 uvSize = glm::vec2{src.width(), src.height()} / gsl::narrow_cast<float>(atlases.getSize());
      static constexpr int YOffset = -ui::FontHeight + 1;
      world::Sprite sprite{atlasLoc.first,
                           uvLoc,
                           uvLoc + uvSize,
                           {0, 0 + YOffset},
                           {src.width(), src.height() + YOffset},
                           nullptr,
                           nullptr};

      std::string btnName;
      if(std::holds_alternative<NamedGlfwGamepadButton>(buttonOrAxis))
      {
        btnName = toString(std::get<NamedGlfwGamepadButton>(buttonOrAxis));
      }
      else if(std::holds_alternative<NamedGlfwAxis>(buttonOrAxis))
      {
        btnName = toString(std::get<NamedGlfwAxis>(buttonOrAxis));
      }
      else
      {
        BOOST_THROW_EXCEPTION(std::runtime_error("invalid controller button configuration"));
      }

      sprite.yBoundMesh = sprite.billboardMesh
        = render::scene::createSpriteMesh(static_cast<float>(sprite.render0.x),
                                          static_cast<float>(-sprite.render0.y),
                                          static_cast<float>(sprite.render1.x),
                                          static_cast<float>(-sprite.render1.y),
                                          sprite.uv0,
                                          sprite.uv1,
                                          material,
                                          sprite.textureId.get_as<int32_t>(),
                                          "controller-" + layout.name + "-" + btnName);

      if(std::holds_alternative<NamedGlfwGamepadButton>(buttonOrAxis))
      {
        currentLayout.emplace(std::get<NamedGlfwGamepadButton>(buttonOrAxis).value, sprite);
      }
      else
      {
        currentLayout.emplace(std::get<NamedGlfwAxis>(buttonOrAxis).value, sprite);
      }
    }

    controllerLayouts.emplace(layout.name, std::move(currentLayout));
  }

  return controllerLayouts;
}
} // namespace engine
