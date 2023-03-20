#include "ghostmanager.h"

#include "cameracontroller.h"
#include "core/i18n.h"
#include "core/units.h"
#include "engine.h"
#include "engine/ghosting/ghostfinishstate.h"
#include "ghosting/ghost.h"
#include "ghosting/ghostmodel.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "location.h"
#include "objectmanager.h"
#include "objects/laraobject.h"
#include "objects/objectstate.h"
#include "presenter.h"
#include "qs/quantity.h"
#include "render/material/materialmanager.h"
#include "render/scene/node.h"
#include "serialization/serialization.h"
#include "serialization/yamldocument.h"
#include "throttler.h"
#include "ui/ui.h"
#include "ui/widgets/messagebox.h"
#include "world/room.h"
#include "world/world.h"
#include "writeonlyxzarchive.h"

#include <boost/log/trivial.hpp>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <sstream>
#include <string>
#include <system_error>

namespace engine
{
GhostManager::GhostManager(const std::filesystem::path& recordingPath, world::World& world)
    : m_model{std::make_shared<ghosting::GhostModel>()}
    , m_readerPath{std::filesystem::path{recordingPath}.replace_extension(".bin")}
    , m_writerPath{recordingPath}
    , m_writer{std::make_unique<ghosting::GhostDataWriter>(recordingPath)}
{
  if(std::filesystem::is_regular_file(m_readerPath))
  {
    m_reader = std::make_unique<ghosting::GhostDataReader>(m_readerPath);
    if(!m_reader->isOpen())
      m_reader.reset();
  }
  if(m_reader != nullptr)
  {
    for(auto i = 0_frame; i < world.getGhostFrame(); i += 1_frame)
    {
      m_writer->append(m_reader->read());
    }
  }
  else
  {
    for(auto i = 0_frame; i < world.getGhostFrame(); i += 1_frame)
    {
      m_writer->append({});
    }
  }
}

GhostManager::~GhostManager()
{
  m_writer.reset();
  std::error_code ec;
  std::filesystem::remove(m_writerPath, ec);
}

bool GhostManager::askGhostSave(Presenter& presenter, world::World& world)
{
  const auto msgBox = std::make_shared<ui::widgets::MessageBox>(
    /* translators: TR charmap encoding */ _("Save recorded ghost?"));
  msgBox->fitToContent();
  msgBox->setConfirmed(false);

  Throttler throttler;
  while(true)
  {
    throttler.wait();

    if(presenter.shouldClose())
    {
      return false;
    }

    if(!presenter.preFrame())
    {
      continue;
    }

    if(presenter.getInputHandler().hasDebouncedAction(hid::Action::Left)
       || presenter.getInputHandler().hasDebouncedAction(hid::Action::Right))
    {
      msgBox->setConfirmed(!msgBox->isConfirmed());
    }

    ui::Ui ui{presenter.getMaterialManager()->getUi(), world.getPalette(), presenter.getUiViewport()};

    msgBox->setPosition({(ui.getSize().x - msgBox->getSize().x) / 2, (ui.getSize().y - msgBox->getSize().y) / 2});
    msgBox->update(true);
    msgBox->draw(ui, presenter);
    {
      const auto portals = world.getCameraController().update();
      if(const auto lara = world.getObjectManager().getLaraPtr())
        lara->m_state.location.room->node->setVisible(true);
      presenter.renderWorld(world.getRooms(), world.getCameraController(), portals, world);
    }
    presenter.renderScreenOverlay();
    presenter.renderUi(ui, 1.0f);
    presenter.updateSoundEngine();
    presenter.swapBuffers();
    if(!presenter.getInputHandler().hasDebouncedAction(hid::Action::Action))
      continue;

    if(!msgBox->isConfirmed())
      return true;

    m_reader.reset();
    m_writer.reset();

    std::error_code ec;
    std::filesystem::remove(m_readerPath, ec);

    std::filesystem::rename(m_writerPath, m_readerPath, ec);

    ghosting::GhostMeta ghostMeta;
    ghostMeta.duration = world.getGhostFrame();
    ghostMeta.level = world.getLevelFilename().stem();
    ghostMeta.gameflow = world.getEngine().getGameflowId();
    if(world.levelFinished())
    {
      ghostMeta.finishState = world.getObjectManager().getLara().m_state.isDead()
                                ? ghosting::GhostFinishState::Death
                                : ghosting::GhostFinishState::Completed;
    }
    else
    {
      ghostMeta.finishState = world.getObjectManager().getLara().m_state.isDead()
                                ? ghosting::GhostFinishState::Death
                                : ghosting::GhostFinishState::Unfinished;
    }

    const auto ymlFilepath = std::filesystem::path{m_readerPath}.replace_extension(".yml");
    serialization::YAMLDocument<false> metaDoc{ymlFilepath};
    metaDoc.serialize("ghost", gsl::not_null{&ghostMeta}, ghostMeta);
    metaDoc.write();

    const auto tarXzFilepath = std::filesystem::path{m_readerPath}.replace_extension(".tar.xz");
    BOOST_LOG_TRIVIAL(debug) << "Create archive " << tarXzFilepath;
    WriteOnlyXzArchive archive{tarXzFilepath};

    auto addFile = [&archive](const std::filesystem::path& filepath)
    {
      BOOST_LOG_TRIVIAL(debug) << "Add archive file " << filepath << " as " << filepath.filename();
      archive.addFile(filepath, filepath.filename());
    };

    addFile(m_readerPath);
    addFile(ymlFilepath);
    return true;
  }
}
} // namespace engine
