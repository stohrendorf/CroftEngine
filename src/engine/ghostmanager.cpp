#include "ghostmanager.h"

#include "cameracontroller.h"
#include "core/i18n.h"
#include "ghosting/ghost.h"
#include "ghosting/ghostmodel.h"
#include "hid/inputhandler.h"
#include "objects/laraobject.h"
#include "presenter.h"
#include "render/scene/materialmanager.h"
#include "serialization/serialization.h"
#include "serialization/yamldocument.h"
#include "throttler.h"
#include "ui/widgets/messagebox.h"
#include "world/world.h"

#include <archive.h>
#include <archive_entry.h>
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
      presenter.renderWorld(world.getRooms(), world.getCameraController(), portals);
    }
    presenter.renderScreenOverlay();
    presenter.renderUi(ui, 1.0f);
    presenter.updateSoundEngine();
    presenter.swapBuffers();
    if(presenter.getInputHandler().hasDebouncedAction(hid::Action::Action))
    {
      if(msgBox->isConfirmed())
      {
        reader.reset();
        writer.reset();

        std::error_code ec;
        std::filesystem::remove(readerPath, ec);

        std::filesystem::rename(writerPath, readerPath, ec);

        ghosting::GhostMeta ghostMeta;
        ghostMeta.duration = world.getGhostFrame();
        ghostMeta.level = world.getLevelFilename().stem();
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
        metaDoc.save("ghost", ghostMeta, ghostMeta);
        metaDoc.write();

        const auto tarXzFilepath = std::filesystem::path{readerPath}.replace_extension(".tar.xz");
        BOOST_LOG_TRIVIAL(debug) << "Create archive " << tarXzFilepath;
        auto a = archive_write_new();
        gsl_Assert(a != nullptr);
        const auto closeArchive = gsl::finally(
          [&a]()
          {
            gsl_Assert(archive_write_close(a) == ARCHIVE_OK);
            gsl_Assert(archive_write_free(a) == ARCHIVE_OK);
          });
        gsl_Assert(archive_write_set_format_pax_restricted(a) == ARCHIVE_OK);
        gsl_Assert(archive_write_add_filter_xz(a) == ARCHIVE_OK);
        gsl_Assert(archive_write_set_options(a, "compression-level=9") == ARCHIVE_OK);
        gsl_Assert(archive_write_open_filename(a, tarXzFilepath.string().c_str()) == ARCHIVE_OK);

        auto addFile = [&a](const std::filesystem::path& filepath)
        {
          BOOST_LOG_TRIVIAL(debug) << "Add archive file " << filepath << " as " << filepath.filename();
          auto entry = archive_entry_new();
          gsl_Assert(entry != nullptr);
          const auto freeEntry = gsl::finally(
            [&entry]()
            {
              archive_entry_free(entry);
            });
          archive_entry_copy_pathname(entry, filepath.filename().string().c_str());
          archive_entry_set_size(entry, std::filesystem::file_size(filepath));
          archive_entry_set_filetype(entry, AE_IFREG);
          archive_entry_set_perm(entry, 0644);
          gsl_Assert(archive_write_header(a, entry) == ARCHIVE_OK);

          std::vector<char> buffer;
          buffer.resize(1024);
          std::ifstream f{filepath, std::ios::in | std::ios::binary};
          gsl_Assert(f.is_open());
          do
          {
            f.read(buffer.data(), buffer.size());
            gsl_Assert(archive_write_data(a, buffer.data(), f.gcount()) == f.gcount());
          } while(f.gcount() > 0);
        };

        addFile(readerPath);
        addFile(ymlFilepath);
      }
      return true;
    }
  }
}
} // namespace engine
