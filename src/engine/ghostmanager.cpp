#include "ghostmanager.h"

#include "cameracontroller.h"
#include "core/i18n.h"
#include "engine.h"
#include "ghosting/ghost.h"
#include "ghosting/ghostmodel.h"
#include "hid/inputhandler.h"
#include "objects/laraobject.h"
#include "presenter.h"
#include "render/material/materialmanager.h"
#include "serialization/serialization.h"
#include "serialization/yamldocument.h"
#include "throttler.h"
#include "ui/widgets/messagebox.h"
#include "world/world.h"
#include "writeonlyxzarchive.h"

#include <boost/log/trivial.hpp>

namespace engine
{
GhostManager::GhostManager(const std::filesystem::path& recordingPath, world::World& world)
    : model{std::make_shared<ghosting::GhostModel>()}
    , readerPath{std::filesystem::path{recordingPath}.replace_extension(".bin")}
    , writerPath{recordingPath}
    , writer{std::make_unique<ghosting::GhostDataWriter>(recordingPath)}
{
  if(std::filesystem::is_regular_file(readerPath))
  {
    reader = std::make_unique<ghosting::GhostDataReader>(readerPath);
    if(!reader->isOpen())
      reader.reset();
  }
  if(reader != nullptr)
  {
    for(auto i = 0_frame; i < world.getGhostFrame(); i += 1_frame)
    {
      writer->append(reader->read());
    }
  }
  else
  {
    for(auto i = 0_frame; i < world.getGhostFrame(); i += 1_frame)
    {
      writer->append({});
    }
  }
}

GhostManager::~GhostManager()
{
  writer.reset();
  std::error_code ec;
  std::filesystem::remove(writerPath, ec);
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

    reader.reset();
    writer.reset();

    std::error_code ec;
    std::filesystem::remove(readerPath, ec);

    std::filesystem::rename(writerPath, readerPath, ec);

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

    const auto ymlFilepath = std::filesystem::path{readerPath}.replace_extension(".yml");
    serialization::YAMLDocument<false> metaDoc{ymlFilepath};
    metaDoc.serialize("ghost", ghostMeta, ghostMeta);
    metaDoc.write();

    const auto tarXzFilepath = std::filesystem::path{readerPath}.replace_extension(".tar.xz");
    BOOST_LOG_TRIVIAL(debug) << "Create archive " << tarXzFilepath;
    WriteOnlyXzArchive archive{tarXzFilepath};

    auto addFile = [&archive](const std::filesystem::path& filepath)
    {
      BOOST_LOG_TRIVIAL(debug) << "Add archive file " << filepath << " as " << filepath.filename();
      archive.addFile(filepath, filepath.filename());
    };

    addFile(readerPath);
    addFile(ymlFilepath);
    return true;
  }
}
} // namespace engine
