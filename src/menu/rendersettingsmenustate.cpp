#include "rendersettingsmenustate.h"

#include "core/i18n.h"
#include "engine/displaysettings.h"
#include "engine/engine.h"
#include "engine/engineconfig.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "hid/inputstate.h"
#include "launcher/networkconfig.h"
#include "menustate.h"
#include "render/rendersettings.h"
#include "selectedmenustate.h"
#include "ui/ui.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/checklistbox.h"
#include "ui/widgets/label.h"
#include "ui/widgets/listbox.h"
#include "ui/widgets/tabbox.h"
#include "ui/widgets/textbox.h"
#include "ui/widgets/valueselector.h"
#include "ui/widgets/widget.h"

#include <boost/format.hpp>
#include <boost/throw_exception.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <gl/glad_init.h>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <ios>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

namespace menu
{
constexpr int MaxDescriptionWidth = 500;

RenderSettingsMenuState::RenderSettingsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                                 std::unique_ptr<MenuState> previous,
                                                 engine::Engine& engine)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_tabs{gsl::make_unique<ui::widgets::TabBox>()}

{
  static const auto toggle = [](engine::Engine& engine, bool& value)
  {
    value = !value;
    engine.applySettings();
  };

  auto tab = gsl::make_shared<ui::widgets::Tab>(/* translators: TR charmap encoding */ _("Effects"));

  auto listBox = gsl::make_shared<ui::widgets::CheckListBox>();
  m_listBoxes.emplace_back(listBox);
  m_tabs->addTab(tab, listBox);
  m_descriptions.emplace_back();

  {
    auto tmp = std::make_shared<ui::widgets::ValueSelector<uint8_t>>(
      [](uint8_t value)
      {
        return /* translators: TR charmap encoding */ _("CRT Version \x1f\x6c %1% \x1f\x6d", value + 1);
      },
      [&engine](uint8_t value)
      {
        engine.getEngineConfig()->renderSettings.crtVersion = value;
        engine.applySettings();
      },
      std::vector<uint8_t>{0, 1});
    listBox->addSetting(
      gslu::nn_shared<ui::widgets::Widget>{tmp},
      [&engine]()
      {
        return engine.getEngineConfig()->renderSettings.crtActive;
      },
      [&engine]()
      {
        toggle(engine, engine.getEngineConfig()->renderSettings.crtActive);
      });
    tmp->selectValue(engine.getEngineConfig()->renderSettings.crtVersion);
    m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
      /* translators: TR charmap encoding */ _(
        "Enables an old TV effect. You can choose between two different effect versions."),
      MaxDescriptionWidth));
  }

  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Depth-of-Field"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.dof;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.dof);
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _(
      "Slightly blurs objects that are out of focus. The focus point is the center of the screen.\n\n"
      "PERFORMANCE: May have slight performance effects on lower-end graphics cards."),
    MaxDescriptionWidth));
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Lens Distortion"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.lensDistortion;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.lensDistortion);
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _(
      "Adds a slight distortion of the graphics to simulate cameras like, for example, a Go Pro."),
    MaxDescriptionWidth));

  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Film Grain"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.filmGrain;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.filmGrain);
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Adds a slight noise effect to simulate analog film."),
    MaxDescriptionWidth));
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Velvia"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.velvia;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.velvia);
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Adds a 'Fujifilm Velvia' effect to enhance colour saturation."),
    MaxDescriptionWidth));
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Bloom"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.bloom;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.bloom);
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _(
      "Bright areas on screen will add a glow to surrounding areas, simulating real-world cameras."),
    MaxDescriptionWidth));
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Edge Outlines"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.edges;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.edges);
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Adds Borderland-like black lines to edges."), MaxDescriptionWidth));
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Muzzle Flash Lighting"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.muzzleFlashLight;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.muzzleFlashLight);
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Firing your weapons will cast light on your environment."),
    MaxDescriptionWidth));

  {
    std::vector<int8_t> values;
    for(int8_t v = -100; v <= 100; v += 5)
      values.emplace_back(v);

    auto tmp = std::make_shared<ui::widgets::ValueSelector<int8_t>>(
      [](int8_t value)
      {
        return /* translators: TR charmap encoding */ _("Brightness \x1f\x6c %1% \x1f\x6d",
                                                        boost::io::group(std::showpos, static_cast<int32_t>(value)));
      },
      [&engine](int8_t value)
      {
        engine.getEngineConfig()->renderSettings.brightness = value;
        engine.applySettings();
      },
      values);
    listBox->addSetting(
      gslu::nn_shared<ui::widgets::Widget>{tmp},
      [&engine]()
      {
        return engine.getEngineConfig()->renderSettings.brightnessEnabled;
      },
      [&engine]()
      {
        toggle(engine, engine.getEngineConfig()->renderSettings.brightnessEnabled);
      });
    tmp->selectValue(engine.getEngineConfig()->renderSettings.brightness);
    m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
      /* translators: TR charmap encoding */ _("Configure your preferred overall brightness of the game."),
      MaxDescriptionWidth));
  }

  {
    std::vector<int8_t> values;
    for(int8_t v = -100; v <= 100; v += 5)
      values.emplace_back(v);

    auto tmp = std::make_shared<ui::widgets::ValueSelector<int8_t>>(
      [](int8_t value)
      {
        return /* translators: TR charmap encoding */ _("Contrast \x1f\x6c %1% \x1f\x6d",
                                                        boost::io::group(std::showpos, static_cast<int32_t>(value)));
      },
      [&engine](int8_t value)
      {
        engine.getEngineConfig()->renderSettings.contrast = value;
        engine.applySettings();
      },
      values);
    listBox->addSetting(
      gslu::nn_shared<ui::widgets::Widget>{tmp},
      [&engine]()
      {
        return engine.getEngineConfig()->renderSettings.contrastEnabled;
      },
      [&engine]()
      {
        toggle(engine, engine.getEngineConfig()->renderSettings.contrastEnabled);
      });
    tmp->selectValue(engine.getEngineConfig()->renderSettings.contrast);
    m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
      /* translators: TR charmap encoding */ _("Configure your preferred overall contrast of the game."),
      MaxDescriptionWidth));
  }

  {
    const std::vector<int32_t> values{1, 2, 3};

    auto tmp = std::make_shared<ui::widgets::ValueSelector<int32_t>>(
      [](int32_t value)
      {
        std::string mode;
        switch(value)
        {
        case 1:
          mode = /* translators: TR charmap encoding */ pgettext("lighting", "Partially");
          break;
        case 2:
          mode = /* translators: TR charmap encoding */ pgettext("lighting", "Mostly");
          break;
        case 3:
          mode = /* translators: TR charmap encoding */ pgettext("lighting", "Full");
          break;
        default:
          BOOST_THROW_EXCEPTION(std::domain_error("invalid lighting mode"));
        }
        return /* translators: TR charmap encoding */ _("\x1f\x6c %1% \x1f\x6d Dynamic Lighting", mode);
      },
      [&engine](int32_t value)
      {
        engine.getEngineConfig()->renderSettings.lightingMode = value;
        engine.applySettings();
      },
      values);
    listBox->addSetting(
      gslu::nn_shared<ui::widgets::Widget>{tmp},
      [&engine]()
      {
        return engine.getEngineConfig()->renderSettings.lightingModeActive;
      },
      [&engine]()
      {
        toggle(engine, engine.getEngineConfig()->renderSettings.lightingModeActive);
      });
    tmp->selectValue(engine.getEngineConfig()->renderSettings.lightingMode);
    m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
      /* translators: TR charmap encoding */ _(
        "Changes the lighting of the level between the static lighting baked into the level data, or dynamic lighting "
        "calculated only from the light sources placed in the levels."),
      MaxDescriptionWidth));
  }

  listBox = gsl::make_shared<ui::widgets::CheckListBox>();
  m_listBoxes.emplace_back(listBox);
  tab = gsl::make_shared<ui::widgets::Tab>(/* translators: TR charmap encoding */ _("Quality"));
  m_descriptions.emplace_back();

  m_tabs->addTab(tab, listBox);

  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Bilinear Filtering"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.bilinearFiltering;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.bilinearFiltering);
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Change between pixelated textures and washed out textures."),
    MaxDescriptionWidth));
  if(gl::hasAnisotropicFilteringExtension())
  {
    const auto maxLevel = gsl::narrow<uint32_t>(std::lround(gl::getMaxAnisotropyLevel()));
    std::vector<uint32_t> levels;
    for(uint32_t i = 1; i <= maxLevel; i *= 2)
    {
      levels.emplace_back(i);
    }

    {
      auto tmp = std::make_shared<ui::widgets::ValueSelector<uint32_t>>(
        [](uint32_t level)
        {
          return /* translators: TR charmap encoding */ _("\x1f\x6c %1%x \x1f\x6d Anisotropic Filtering", level);
        },
        [&engine](uint32_t value)
        {
          engine.getEngineConfig()->renderSettings.anisotropyLevel = value;
          engine.applySettings();
        },
        std::move(levels));
      tmp->selectValue(engine.getEngineConfig()->renderSettings.anisotropyLevel);

      listBox->addSetting(
        gslu::nn_shared<ui::widgets::Widget>{tmp},
        [&engine]()
        {
          return engine.getEngineConfig()->renderSettings.anisotropyActive;
        },
        [&engine, maxLevel = gsl::narrow<uint32_t>(std::lround(gl::getMaxAnisotropyLevel()))]()
        {
          toggle(engine, engine.getEngineConfig()->renderSettings.anisotropyActive);
        });
    }
    m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
      /* translators: TR charmap encoding */ _("Improves texture rendering of slanted geometry."),
      MaxDescriptionWidth));
  }
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Denoise Water Surface"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.waterDenoise;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.waterDenoise);
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Reduces the choppiness of the water surface."), MaxDescriptionWidth));
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("HBAO"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.hbao;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.hbao);
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _(
      "Adds shadows to crevices.\n\n"
      "PERFORMANCE: Has a mild effect on performance. Disable if you experience constant low framerates."),
    MaxDescriptionWidth));

  {
    auto tmp = std::make_shared<ui::widgets::ValueSelector<uint8_t>>(
      [](uint8_t value)
      {
        const char* quality = nullptr;
        switch(value)
        {
        case 10:
          quality = pgettext("FXAA", "Low");
          break;
        case 15:
          quality = pgettext("FXAA", "Medium");
          break;
        case 29:
          quality = pgettext("FXAA", "High");
          break;
        case 39:
          quality = pgettext("FXAA", "Extreme");
          break;
        default:
          BOOST_THROW_EXCEPTION(std::out_of_range("invalid fxaa preset"));
        }
        return /* translators: TR charmap encoding */ _("FXAA \x1f\x6c %1% \x1f\x6d", quality);
      },
      [&engine](uint8_t value)
      {
        engine.getEngineConfig()->renderSettings.fxaaPreset = value;
        engine.applySettings();
      },
      std::vector<uint8_t>{10, 15, 29, 39});
    listBox->addSetting(
      gslu::nn_shared<ui::widgets::Widget>{tmp},
      [&engine]()
      {
        return engine.getEngineConfig()->renderSettings.fxaaActive;
      },
      [&engine]()
      {
        toggle(engine, engine.getEngineConfig()->renderSettings.fxaaActive);
      });
    tmp->selectValue(engine.getEngineConfig()->renderSettings.fxaaPreset);
    m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
      /* translators: TR charmap encoding */ _(
        "Smoothes out rendering edges.\n\n"
        "PERFORMANCE: Has a mild effect on performance. Disable if you experience constant low framerates."),
      MaxDescriptionWidth));
  }

  listBox->addSetting(
    /* translators: TR charmap encoding */ _("High Quality Shadows"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.highQualityShadows;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.highQualityShadows);
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _(
      "Switch between high and low resolution shadow maps.\n\n"
      "PERFORMANCE: Has a mild effect on performance. Disable if you experience constant low framerates."),
    MaxDescriptionWidth));

  {
    auto tmp = std::make_shared<ui::widgets::ValueSelector<uint8_t>>(
      [](uint8_t value)
      {
        return /* translators: TR charmap encoding */ _("\x1f\x6c 1/%1% \x1f\x6d Render Scale",
                                                        static_cast<uint32_t>(value));
      },
      [&engine](uint8_t value)
      {
        engine.getEngineConfig()->renderSettings.renderResolutionDivisor = value;
        engine.applySettings();
      },
      std::vector<uint8_t>{2, 3, 4, 5, 6, 7, 8});
    listBox->addSetting(
      gslu::nn_shared<ui::widgets::Widget>{tmp},
      [&engine]()
      {
        return engine.getEngineConfig()->renderSettings.renderResolutionDivisorActive;
      },
      [&engine]()
      {
        toggle(engine, engine.getEngineConfig()->renderSettings.renderResolutionDivisorActive);
      });
    tmp->selectValue(engine.getEngineConfig()->renderSettings.renderResolutionDivisor);
  }
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Reduces render scale to lower resolutions.\n\n"
                                             "PERFORMANCE: Can greatly improve performance."),
    MaxDescriptionWidth));

  {
    auto tmp = std::make_shared<ui::widgets::ValueSelector<uint8_t>>(
      [](uint8_t value)
      {
        return /* translators: TR charmap encoding */ _("\x1f\x6c %1%x \x1f\x6d UI Scale",
                                                        static_cast<uint32_t>(value));
      },
      [&engine](uint8_t value)
      {
        engine.getEngineConfig()->renderSettings.uiScaleMultiplier = value;
        engine.applySettings();
      },
      std::vector<uint8_t>{2, 3, 4, 5, 6, 7, 8});
    listBox->addSetting(
      gslu::nn_shared<ui::widgets::Widget>{tmp},
      [&engine]()
      {
        return engine.getEngineConfig()->renderSettings.uiScaleActive;
      },
      [&engine]()
      {
        toggle(engine, engine.getEngineConfig()->renderSettings.uiScaleActive);
      });
    tmp->selectValue(engine.getEngineConfig()->renderSettings.uiScaleMultiplier);
  }
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Scales the UI text."), MaxDescriptionWidth));

  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Smoother Animations"),
    [&engine]()
    {
      return engine.getEngineConfig()->animSmoothing;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->animSmoothing);
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Smoothes out some choppy model animations."), MaxDescriptionWidth));

  listBox = gsl::make_shared<ui::widgets::CheckListBox>();
  m_listBoxes.emplace_back(listBox);
  tab = gsl::make_shared<ui::widgets::Tab>(/* translators: TR charmap encoding */ _("Other"));
  m_tabs->addTab(tab, listBox);
  m_descriptions.emplace_back();

  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Fullscreen"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.fullscreen;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.fullscreen);
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Enables windowed fullscreen."), MaxDescriptionWidth));

  {
    auto tmp = std::make_shared<ui::widgets::ValueSelector<uint8_t>>(
      [](uint8_t value)
      {
        return /* translators: TR charmap encoding */ _("\x1f\x6c %1% \x1f\x6d Percent Dust Particles", 100 / value);
      },
      [&engine](uint8_t value)
      {
        engine.getEngineConfig()->renderSettings.dustDensity = value;
        engine.applySettings();
      },
      std::vector<uint8_t>{10, 5, 4, 3, 2, 1});
    listBox->addSetting(
      gslu::nn_shared<ui::widgets::Widget>{tmp},
      [&engine]()
      {
        return engine.getEngineConfig()->renderSettings.dustActive;
      },
      [&engine]()
      {
        toggle(engine, engine.getEngineConfig()->renderSettings.dustActive);
      });
    tmp->selectValue(engine.getEngineConfig()->renderSettings.dustDensity);
    m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
      /* translators: TR charmap encoding */ _(
        "Adds small dust particles to all levels.\n\n"
        "PERFORMANCE: Has a big effect on performance. Disable if you experience low framerates in large, open areas."),
      MaxDescriptionWidth));
  }

  listBox->addSetting(
    /* translators: TR charmap encoding */ _("More Lights"),
    [&engine]()
    {
      return engine.getEngineConfig()->renderSettings.moreLights;
    },
    [&engine]()
    {
      toggle(engine, engine.getEngineConfig()->renderSettings.moreLights);
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Makes lighting changes between areas of levels smoother.\n\n"
                                             "This adds lights of neighbouring areas to the lighting calculation. "
                                             "Disable if the game becomes too bright for you."),
    MaxDescriptionWidth));
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Ghost"),
    [&engine]()
    {
      return engine.getEngineConfig()->displaySettings.ghost;
    },
    [&engine]()
    {
      auto& b = engine.getEngineConfig()->displaySettings.ghost;
      b = !b;
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Enables recording and playback of your local ghost."),
    MaxDescriptionWidth));
  if(launcher::NetworkConfig::load().isValid())
  {
    listBox->addSetting(
      /* translators: TR charmap encoding */ _("Show Players' Names"),
      [&engine]()
      {
        return engine.getEngineConfig()->displaySettings.showCoopNames;
      },
      [&engine]()
      {
        auto& b = engine.getEngineConfig()->displaySettings.showCoopNames;
        b = !b;
      });
    m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
      /* translators: TR charmap encoding */ _("Shows other player names when enabled."), MaxDescriptionWidth));
  }
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Restore Health on Level Start"),
    [&engine]()
    {
      return engine.getEngineConfig()->restoreHealth;
    },
    [&engine]()
    {
      auto& b = engine.getEngineConfig()->restoreHealth;
      b = !b;
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Enable to restore your health on each level start. When disabled, your"
                                             " health won't get filled up after each level."),
    MaxDescriptionWidth));
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Low Health: Pulse Health Bar"),
    [&engine]()
    {
      return engine.getEngineConfig()->pulseLowHealthHealthBar;
    },
    [&engine]()
    {
      auto& b = engine.getEngineConfig()->pulseLowHealthHealthBar;
      b = !b;
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _(
      "Enable to make the health bar pulse on low health. The lower your health, the faster it will pulse."),
    MaxDescriptionWidth));
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Low Health: Reduce Color"),
    [&engine]()
    {
      return engine.getEngineConfig()->lowHealthMonochrome;
    },
    [&engine]()
    {
      auto& b = engine.getEngineConfig()->lowHealthMonochrome;
      b = !b;
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Enable to make the screen go black and white the lower your health is. "
                                             "This effect will only start to take effect on very low health."),
    MaxDescriptionWidth));
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Water Bed Bubbles"),
    [&engine]()
    {
      return engine.getEngineConfig()->waterBedBubbles;
    },
    [&engine]()
    {
      auto& b = engine.getEngineConfig()->waterBedBubbles;
      b = !b;
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Adds small bubbles to all water."), MaxDescriptionWidth));
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Butt Bubbles"),
    [&engine]()
    {
      return engine.getEngineConfig()->buttBubbles;
    },
    [&engine]()
    {
      auto& b = engine.getEngineConfig()->buttBubbles;
      b = !b;
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Give Lara a bubbly personality."), MaxDescriptionWidth));
  {
    auto tmp = std::make_shared<ui::widgets::ValueSelector<uint8_t>>(
      [](uint8_t value)
      {
        return /* translators: TR charmap encoding */ P_("Save Reminder after \x1f\x6c %1% \x1f\x6d Minute",
                                                         "Save Reminder after \x1f\x6c %1% \x1f\x6d Minutes",
                                                         value,
                                                         static_cast<uint32_t>(value));
      },
      [&engine](uint8_t value)
      {
        engine.getEngineConfig()->saveReminderMinutes = value;
      },
      std::vector<uint8_t>{1, 2, 3, 4, 5, 10, 15, 20, 25, 30, 45, 60});
    listBox->addSetting(
      gslu::nn_shared<ui::widgets::Widget>{tmp},
      [&engine]()
      {
        return engine.getEngineConfig()->saveReminderEnabled;
      },
      [&engine]()
      {
        toggle(engine, engine.getEngineConfig()->saveReminderEnabled);
      });
    tmp->selectValue(engine.getEngineConfig()->saveReminderMinutes);
    m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
      /* translators: TR charmap encoding */ _("Adds a regular save reminder to the top of the screen."),
      MaxDescriptionWidth));
  }
  listBox->addSetting(
    /* translators: TR charmap encoding */ _("Select First Free or Oldest Slot"),
    [&engine]()
    {
      return engine.getEngineConfig()->selectFirstFreeOrOldestSlot;
    },
    [&engine]()
    {
      auto& b = engine.getEngineConfig()->selectFirstFreeOrOldestSlot;
      b = !b;
    });
  m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _(
      "When saving manually, automatically select the first free slot. If no more free slots are available, it will "
      "select the oldest save to be overwritten."),
    MaxDescriptionWidth));
  {
    auto tmp = std::make_shared<ui::widgets::ValueSelector<uint8_t>>(
      [](uint8_t value)
      {
        return /* translators: TR charmap encoding */ P_("Delay Overwrite for \x1f\x6c %1% \x1f\x6d Second",
                                                         "Delay Overwrite for \x1f\x6c %1% \x1f\x6d Seconds",
                                                         value,
                                                         static_cast<uint32_t>(value));
      },
      [&engine](uint8_t value)
      {
        engine.getEngineConfig()->delaySaveDurationSeconds = value;
      },
      std::vector<uint8_t>{1, 2, 3, 4, 5});
    listBox->addSetting(
      gslu::nn_shared<ui::widgets::Widget>{tmp},
      [&engine]()
      {
        return engine.getEngineConfig()->delaySaveEnabled;
      },
      [&engine]()
      {
        toggle(engine, engine.getEngineConfig()->delaySaveEnabled);
      });
    tmp->selectValue(engine.getEngineConfig()->delaySaveDurationSeconds);
    m_descriptions.back().emplace_back(std::make_shared<ui::widgets::TextBox>(
      /* translators: TR charmap encoding */ _(
        "When manually overwriting a save slot, you will need to confirm 'Yes' for "
        "some time to avoid accidental overwrites."),
      MaxDescriptionWidth));
  }

  for(const auto& descriptions : m_descriptions)
  {
    for(const auto& description : descriptions)
    {
      description->fitToContent();
      description->setSize({MaxDescriptionWidth, description->getSize().y});
    }
  }
}

std::unique_ptr<MenuState>
  RenderSettingsMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& /*display*/)
{
  m_tabs->fitToContent();
  m_tabs->setPosition({(ui.getSize().x - m_tabs->getSize().x) / 2, ui.getSize().y - m_tabs->getSize().y - 90});

  for(size_t i = 0; i < m_listBoxes.size(); ++i)
  {
    m_listBoxes[i]->update(i == m_tabs->getSelectedTab());
  }
  m_tabs->update(true);
  m_tabs->draw(ui, world.getPresenter());

  const auto& listBox = m_listBoxes[m_tabs->getSelectedTab()];

  {
    const auto& descriptions = m_descriptions.at(m_tabs->getSelectedTab());
    const auto& description = descriptions.at(listBox->getSelectedIndex());
    description->setPosition({(ui.getSize().x - MaxDescriptionWidth) / 2,
                              listBox->getPosition().y - description->getSize().y - 3 * ui::FontHeight});
    description->draw(ui, world.getPresenter());
  }

  if(world.getPresenter().getInputHandler().getInputState().menuZMovement.justChangedTo(hid::AxisMovement::Forward))
  {
    if(!listBox->prevEntry())
    {
      listBox->setSelectedEntry(listBox->getEntryCount() - 1);
    }
  }
  else if(world.getPresenter().getInputHandler().getInputState().menuZMovement.justChangedTo(
            hid::AxisMovement::Backward))
  {
    if(!listBox->nextEntry())
    {
      listBox->setSelectedEntry(0);
    }
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::PrevScreen))
  {
    m_tabs->prevTab();
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::NextScreen))
  {
    m_tabs->nextTab();
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::PrimaryInteraction))
  {
    const auto& [getter, toggler, checkbox] = listBox->getSelected();
    toggler();
    checkbox->setChecked(getter());
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Return))
  {
    return std::move(m_previous);
  }
  else if(auto tmp = gslu::dynamic_pointer_cast<ui::widgets::ValueSelectorBase>(
            std::get<2>(listBox->getSelected())->getContent());
          tmp != nullptr)
  {
    if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::MenuLeft))
      tmp->selectPrev();
    else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::MenuRight))
      tmp->selectNext();
  }

  return nullptr;
}
RenderSettingsMenuState::~RenderSettingsMenuState() = default;
} // namespace menu
