#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN
#endif

// --- FIXME: for whatever reason, compilation fails if this isn't included before mainwindow.h
#include <ryml.hpp>
#include <ryml_std.hpp>
// ---
#include "discfs.h"
#include "discimage.h"
#include "downloadprogress.h"
#include "gameflow/meta.h"
#include "languages.h"
#include "mainwindow.h"
#include "networkconfig.h"
#include "paths.h"
#include "readonlyarchive.h"
#include "serialization/serialization.h"
#include "serialization/yamldocument.h"
#include "ui_mainwindow.h"

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <chrono>
#include <QColorDialog>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>
#include <set>

#ifdef WIN32
#  ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4456)
#  endif
#  include "vdf_parser.hpp"
#  ifdef _MSC_VER
#    pragma warning(pop)
#  endif

#  include <windows.h>
#endif

using namespace std::chrono_literals;

namespace launcher
{
namespace
{
const char* const LanguageConfigKey = "launcher-language";
const char* const GameflowConfigKey = "launcher-gameflow";
const int IdRole = Qt::UserRole + 1;
const int AuthorRole = Qt::UserRole + 2;
const int UrlsRole = Qt::UserRole + 3;

void extractImage(const std::filesystem::path& cueFile, const std::filesystem::path& targetDir)
{
  auto img = std::make_unique<image::DiscImage>(cueFile);
  for(const auto& [path, span] : image::getFiles(*img))
  {
    gsl_Assert(!path.empty());
    const auto root = *path.begin();
    if(root != "DATA" && root != "FMV")
    {
      BOOST_LOG_TRIVIAL(info) << "Skipping root " << root;
      continue;
    }
    else
    {
      BOOST_LOG_TRIVIAL(info) << "Extracting " << path << " to " << (targetDir / path) << " from " << cueFile;
      std::filesystem::create_directories(targetDir / path.parent_path());
      const auto data = image::readFile(*img, span);
      std::ofstream tmp{targetDir / path, std::ios::binary | std::ios::trunc};
      tmp.write(reinterpret_cast<const char*>(data.data()), gsl::narrow<std::streamsize>(data.size()));
    }
  }
}

#ifdef WIN32
std::optional<std::filesystem::path> readRegistryPath(const std::wstring& path, const std::wstring& key)
{
  HKEY subKey;
  if(RegOpenKeyExW(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &subKey) != 0)
  {
    RegCloseKey(subKey);
    return std::nullopt;
  }

  std::array<WCHAR, 512> szBuffer{};
  auto dwBufferSize = gsl::narrow<DWORD>(szBuffer.size());
  DWORD type = REG_NONE;
  const auto nError
    = RegQueryValueExW(subKey, key.c_str(), nullptr, &type, reinterpret_cast<PBYTE>(szBuffer.data()), &dwBufferSize);
  RegCloseKey(subKey);

  if(nError != 0 || type != REG_SZ)
  {
    return std::nullopt;
  }
  return std::filesystem::path{szBuffer.data()};
}
#endif
} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  connect(ui->validateConnection, &QPushButton::clicked, this, &MainWindow::onTestConnectionClicked);

  const QSettings settings;

  {
    const auto language = settings.value(LanguageConfigKey, QVariant{QString{"en_GB"}}).toString();

    ui->languages->setModel(&m_languages);
    m_languages.insertRows(0, gsl::narrow<int>(getSupportedLanguages().size()));
    int i = 0;
    for(const auto& [languageId, languageName] : getSupportedLanguages())
    {
      auto item = new QStandardItem{QString::fromLatin1(languageName.c_str())};
      item->setData(QVariant{QString::fromLatin1(languageId.c_str())}, IdRole);
      m_languages.setItem(i, item);

      if(language == QString::fromLatin1(languageId.c_str()))
        ui->languages->setCurrentIndex(i);
      ++i;
    }
    m_languages.sort(0);
  }

  {
    connect(ui->gameflows, &QListView::clicked, this, &MainWindow::onGameflowSelected);

    const auto gameflow = settings.value(GameflowConfigKey, QVariant{QString{"tr1"}}).toString();

    ui->gameflows->setModel(&m_gameflows);

    std::vector<std::tuple<std::string, gameflow::Meta>> metas;
    const std::filesystem::directory_iterator end{};
    for(std::filesystem::directory_iterator it{findEngineDataDir().value() / "gameflows"}; it != end; ++it)
    {
      if(!std::filesystem::is_regular_file(it->path() / "meta.yml"))
        continue;

      serialization::YAMLDocument<true> doc{it->path() / "meta.yml"};
      gameflow::Meta meta{};
      doc.deserialize("meta", gsl::not_null{&meta}, meta);
      metas.emplace_back(it->path().stem().string(), meta);
    }

    m_gameflows.insertRows(0, gsl::narrow<int>(metas.size()));
    int i = 0;
    for(const auto& [gameflowId, gameflowMeta] : metas)
    {
      auto item = new QStandardItem{QString::fromLatin1(gameflowMeta.title.c_str())};
      item->setData(QVariant{QString::fromLatin1(gameflowId.c_str())}, IdRole);
      item->setData(QVariant{QString::fromLatin1(gameflowMeta.author.c_str())}, AuthorRole);
      item->setEditable(false);

      QStringList urls;
      for(const auto& url : gameflowMeta.urls)
        urls << QString::fromLatin1(url.c_str());

      item->setData(QVariant{urls}, UrlsRole);
      m_gameflows.setItem(i, item);

      ++i;
    }

    m_gameflows.sort(0);

    for(i = 0; i < m_gameflows.rowCount(); ++i)
    {
      const auto index = m_gameflows.index(i, 0);
      if(gameflow == m_gameflows.data(index, IdRole).toString())
      {
        ui->gameflows->setCurrentIndex(index);
        onGameflowSelected(index);
        break;
      }
    }
  }

  connect(ui->btnLaunch, &QPushButton::clicked, this, &MainWindow::onLaunchClicked);

  if(!findUserDataDir().has_value())
  {
    bool canWriteLocalDir;
    {
      auto testPath = (getExpectedLocalUserDataDir() / "deleteme.txt").string();
      auto x = QFile{testPath.c_str()};
      canWriteLocalDir = x.open(QFile::OpenModeFlag::Append);
      x.close();
      QFile::remove(testPath.c_str());
    }

    if(canWriteLocalDir)
    {
      QMessageBox askDataLocation;
      askDataLocation.setWindowTitle(tr("Initial Setup"));
      askDataLocation.setText(tr("Data Folder Not Found"));
      askDataLocation.setInformativeText(
        tr("It seems you're running CroftEngine for the first time. Please select where you want to store your game "
           "data."));
      const auto usePortableBtn = askDataLocation.addButton(tr("Portable"), QMessageBox::ButtonRole::AcceptRole);
      const auto useHomeDirBtn
        = askDataLocation.addButton(tr("In my Home Directory"), QMessageBox::ButtonRole::AcceptRole);
      askDataLocation.addButton("Abort", QMessageBox::ButtonRole::RejectRole);
      askDataLocation.setIcon(QMessageBox::Icon::Question);
      askDataLocation.exec();
      if(askDataLocation.clickedButton() == usePortableBtn)
      {
        std::filesystem::create_directories(getExpectedLocalUserDataDir() / "data");
      }
      else if(askDataLocation.clickedButton() == useHomeDirBtn)
      {
        std::filesystem::create_directories(getExpectedSysUserDataDir() / "data");
      }
      else
      {
        std::exit(EXIT_SUCCESS);
      }
    }
    else
    {
      std::filesystem::create_directories(getExpectedSysUserDataDir() / "data");
      QMessageBox::information(
        this, tr("Data Folder Created"), tr("A game data folder was created in your home directory."));
    }
  }

  ui->engineVersion->setText(QString::fromLatin1(CE_VERSION));
  ui->dataLocation->setText(QString::fromUtf8(findUserDataDir().value().string().c_str()));
  ui->engineDataLocation->setText(QString::fromLatin1(findEngineDataDir().value().string().c_str()));

  connect(ui->openDataLocation, &QPushButton::clicked, this, &MainWindow::onOpenDataLocationClicked);
  connect(ui->migrateBtn, &QPushButton::clicked, this, &MainWindow::onMigrateClicked);
  connect(ui->resetConfig, &QPushButton::clicked, this, &MainWindow::resetConfig);
  connect(ui->selectGlidos, &QPushButton::clicked, this, &MainWindow::onSelectGlidosClicked);
  connect(ui->disableGlidos, &QPushButton::clicked, this, &MainWindow::onDisableGlidosClicked);
  connect(ui->btnChooseColor, &QPushButton::clicked, this, &MainWindow::onChooseColorClicked);

  if(std::filesystem::is_regular_file(findUserDataDir().value() / "network.yaml"))
  {
    auto cfg = NetworkConfig::load();

    m_ghostColor = QColor::fromRgb(cfg.color.at(0), cfg.color.at(1), cfg.color.at(2));
    ui->serverSocket->setText(QString::fromLatin1(cfg.socket.c_str()));
    ui->username->setText(QString::fromUtf8(cfg.username.c_str()));
    ui->authToken->setText(QString::fromUtf8(cfg.authToken.c_str()));
    ui->sessionId->setText(QString::fromUtf8(cfg.sessionId.c_str()));
  }
  else
  {
    // NOLINTNEXTLINE(cert-msc50-cpp)
    m_ghostColor = QColor::fromRgb(std::rand() % 256, std::rand() % 256, std::rand() % 256);
  }

  QPalette pal;
  pal.setColor(QPalette::Window, m_ghostColor);
  ui->lblColor->setPalette(pal);
  ui->lblColor->setAutoFillBackground(true);

  connect(ui->btnCleanUp, &QPushButton::clicked, this, &MainWindow::onCleanUpClicked);
  connect(ui->btnOpenCrashFolder, &QPushButton::clicked, this, &MainWindow::onOpenCrashdumpsClicked);
  ui->crashdumps->setModel(&m_crashdumps);
  populateCrashdumpsList();
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::onOpenDataLocationClicked()
{
  QDesktopServices::openUrl("file:///" + QString::fromUtf8(findUserDataDir().value().string().c_str()));
}

void MainWindow::onImportClicked()
{
  const auto gameflow = ui->gameflows->currentIndex().data(IdRole).toString();
  if(gameflow == "tr1" || gameflow == "tr1ub")
  {
    if(!importBaseGameData())
      return;

    QMessageBox::information(this, tr("Data Imported"), tr("Game Data has been imported."));

    const auto userDataDir = findUserDataDir().value();
    if(std::filesystem::is_regular_file(userDataDir / "data" / "tr1" / "AUDIO" / "002.ogg")
       || std::filesystem::is_regular_file(userDataDir / "data" / "tr1" / "Music" / "Track02.flac"))
      return;

    auto downloader = new DownloadProgress(
      QUrl{"https://opentomb.earvillage.net/croftengine-audio-tr1.zip"}, userDataDir / "data" / "tracks.zip", this);
    connect(downloader, &DownloadProgress::downloaded, this, &MainWindow::extractSoundtrackZip);
    downloader->show();
    downloader->start();
  }
  else if(gameflow == "tr1demo-part1" || gameflow == "tr1demo-part2")
  {
    const auto fileName
      = QFileDialog::getOpenFileName(this, tr("Select Archive"), QString{}, tr("ZIP Archive (*.zip)"));
    if(fileName.isEmpty())
      return;

    std::filesystem::path suspect;
    {
      std::set<std::filesystem::path> archiveDirs;

      ReadOnlyArchive archive{fileName.toStdString()};
      if(archive.failure())
      {
        QMessageBox::critical(
          this,
          tr("Extraction Error"),
          tr("Could not open %1 as an archive: %2").arg(fileName, archive.getErrorString()->c_str()));
        return;
      }

      {
        BOOST_LOG_TRIVIAL(debug) << "gather archive directories";
        while(archive.next())
        {
          switch(archive.getType())
          {
          case ReadOnlyArchive::EntryType::Directory:
            archiveDirs.emplace(archive.getCurrentPathName());
            break;
          case ReadOnlyArchive::EntryType::File:
            archiveDirs.emplace(archive.getCurrentPathName().parent_path());
            break;
          default:
            BOOST_LOG_TRIVIAL(warning) << "unexpected archive entry filetype";
            continue;
          }
        }
      }

      std::map<std::filesystem::path, std::vector<std::filesystem::path>> candidates;
      for(const auto& dir : archiveDirs)
      {
        const auto stem = QString::fromUtf8(dir.stem().string().c_str()).toLower();
        if(stem != "data")
          continue;

        BOOST_LOG_TRIVIAL(debug) << "candidate path " << dir;
        candidates[dir.parent_path()].emplace_back(dir);
      }
      bool foundSuspect = false;
      for(const auto& [base, full] : candidates)
      {
        BOOST_LOG_TRIVIAL(debug) << "suspect path " << base;
        if(foundSuspect)
        {
          QMessageBox::critical(this, tr("Extraction Error"), tr("Could not find game data in archive."));
          return;
        }
        suspect = base;
        foundSuspect = true;
      }
      if(!foundSuspect)
      {
        QMessageBox::critical(this, tr("Extraction Error"), tr("Could not find game data in archive."));
        return;
      }
    }
    {
      ReadOnlyArchive archive{fileName.toStdString()};
      if(archive.failure())
      {
        QMessageBox::critical(this,
                              tr("Extraction Error"),
                              tr("Could not open %1 as an archive: %2")
                                .arg(fileName, archive.getErrorString().value_or("Unknown error").c_str()));
        return;
      }

      const auto dataRoot = findUserDataDir().value() / "data" / gameflow.toStdString();

      while(archive.next())
      {
        {
          bool valid = false;
          for(auto p = archive.getCurrentPathName(); !p.empty(); p = p.parent_path())
          {
            valid |= p.parent_path() == suspect && QString::fromUtf8(p.stem().string().c_str()).toLower() == "data";
            if(valid)
            {
              break;
            }
          }

          if(!valid)
            continue;
        }

        const auto dstName = dataRoot / std::filesystem::relative(archive.getCurrentPathName(), suspect);

        BOOST_LOG_TRIVIAL(debug) << "extract " << archive.getCurrentPathName() << " to " << dstName;

        std::filesystem::create_directories(dstName.parent_path());
        archive.writeCurrentTo(dstName);
      }

      QMessageBox::information(this, tr("Data Imported"), tr("Game Data has been imported."));

      const auto userDataDir = findUserDataDir().value();
      if(std::filesystem::is_regular_file(userDataDir / "data" / gameflow.toStdString() / "AUDIO" / "002.ogg"))
        return;

      auto downloader = new DownloadProgress(
        QUrl{"https://opentomb.earvillage.net/croftengine-audio-tr1.zip"}, userDataDir / "data" / "tracks.zip", this);
      connect(downloader, &DownloadProgress::downloaded, this, &MainWindow::extractSoundtrackZip);
      downloader->show();
      downloader->start();
    }
  }
  else
  {
    const auto fileName
      = QFileDialog::getOpenFileName(this, tr("Select Archive"), QString{}, tr("ZIP Archive (*.zip)"));
    if(fileName.isEmpty())
      return;

    std::filesystem::path suspect;
    {
      std::set<std::filesystem::path> archiveDirs;

      ReadOnlyArchive archive{fileName.toStdString()};
      if(archive.failure())
      {
        QMessageBox::critical(
          this,
          tr("Extraction Error"),
          tr("Could not open %1 as an archive: %2").arg(fileName, archive.getErrorString()->c_str()));
        return;
      }

      {
        BOOST_LOG_TRIVIAL(debug) << "gather archive directories";
        while(archive.next())
        {
          switch(archive.getType())
          {
          case ReadOnlyArchive::EntryType::Directory:
            archiveDirs.emplace(archive.getCurrentPathName());
            break;
          case ReadOnlyArchive::EntryType::File:
            archiveDirs.emplace(archive.getCurrentPathName().parent_path());
            break;
          default:
            BOOST_LOG_TRIVIAL(warning) << "unexpected archive entry filetype";
            continue;
          }
        }
      }

      std::map<std::filesystem::path, std::vector<std::filesystem::path>> candidates;
      for(const auto& dir : archiveDirs)
      {
        const auto stem = QString::fromUtf8(dir.stem().string().c_str()).toLower();
        if(stem != "music" && stem != "audio" && stem != "fmv" && stem != "data")
          continue;

        BOOST_LOG_TRIVIAL(debug) << "candidate path " << dir;
        candidates[dir.parent_path()].emplace_back(dir);
      }
      bool foundSuspect = false;
      for(const auto& [base, full] : candidates)
      {
        if(full.size() < 3)
          continue;
        BOOST_LOG_TRIVIAL(debug) << "suspect path " << base;
        if(foundSuspect)
        {
          QMessageBox::critical(this, tr("Extraction Error"), tr("Could not find game data in archive."));
          return;
        }
        suspect = base;
        foundSuspect = true;
      }
      if(!foundSuspect)
      {
        QMessageBox::critical(this, tr("Extraction Error"), tr("Could not find game data in archive."));
        return;
      }
    }
    {
      ReadOnlyArchive archive{fileName.toStdString()};
      if(archive.failure())
      {
        QMessageBox::critical(this,
                              tr("Extraction Error"),
                              tr("Could not open %1 as an archive: %2")
                                .arg(fileName, archive.getErrorString().value_or("Unknown error").c_str()));
        return;
      }

      const auto dataRoot = findUserDataDir().value() / "data" / gameflow.toStdString();

      while(archive.next())
      {
        {
          bool valid = false;
          for(auto p = archive.getCurrentPathName(); !p.empty(); p = p.parent_path())
          {
            for(const auto stem : {"music", "audio", "fmv", "data"})
            {
              if(p.parent_path() == suspect && QString::fromUtf8(p.stem().string().c_str()).toLower() == stem)
              {
                valid = true;
                break;
              }
            }
          }

          if(!valid)
            continue;
        }

        const auto dstName = dataRoot / std::filesystem::relative(archive.getCurrentPathName(), suspect);

        BOOST_LOG_TRIVIAL(debug) << "extract " << archive.getCurrentPathName() << " to " << dstName;

        std::filesystem::create_directories(dstName.parent_path());
        archive.writeCurrentTo(dstName);
      }

      QMessageBox::information(this, tr("Data Imported"), tr("Game Data has been imported."));
    }
  }
}

namespace
{
#ifdef WIN32
std::optional<std::filesystem::path> tryGetSteamGamePath(const std::filesystem::path& testPath)
{
  const auto tryGetLibraryFolders = [](const std::wstring& path) -> std::vector<std::filesystem::path>
  {
    const auto installPath = readRegistryPath(path, L"InstallPath");
    if(!installPath.has_value())
    {
      BOOST_LOG_TRIVIAL(debug) << "Steam InstallPath not found in registry";
      return {};
    }

    const auto libraryFolderVdfPath = *installPath / "steamapps" / "libraryfolders.vdf";
    if(!std::filesystem::is_regular_file(libraryFolderVdfPath))
    {
      BOOST_LOG_TRIVIAL(debug) << "libraryfolders.vdf not found";
      return {};
    }

    std::ifstream vdf{libraryFolderVdfPath};
    auto root = tyti::vdf::read(vdf);
    if(root.name != "libraryfolders")
    {
      BOOST_LOG_TRIVIAL(debug) << "Invalid libraryfolders.vdf";
      return {};
    }

    std::vector<std::filesystem::path> paths;
    for(const auto& [entryId, entryContent] : root.childs)
    {
      if(std::any_of(entryId.begin(),
                     entryId.end(),
                     [](auto c)
                     {
                       return c < '0' || c > '9';
                     }))
      {
        BOOST_LOG_TRIVIAL(debug) << "Invalid library folder entry key";
        continue;
      }

      if(auto it = entryContent->attribs.find("path"); it != entryContent->attribs.end())
      {
        BOOST_LOG_TRIVIAL(debug) << "Found library folder " << it->second;
        paths.emplace_back(it->second);
      }
      else
      {
        BOOST_LOG_TRIVIAL(debug) << "Incomplete libraryfolders entry content";
      }
    }

    return paths;
  };

  auto libraryFolders = tryGetLibraryFolders(LR"(SOFTWARE\WOW6432Node\Valve\Steam)");
  if(libraryFolders.empty())
    libraryFolders = tryGetLibraryFolders(LR"(SOFTWARE\Valve\Steam)");
  if(libraryFolders.empty())
    return {};

  for(const auto& libFolder : libraryFolders)
  {
    const auto appManifestPath = libFolder / "steamapps" / "appmanifest_224960.acf";
    BOOST_LOG_TRIVIAL(debug) << "Check manifest: " << appManifestPath.string().c_str();
    if(!std::filesystem::is_regular_file(appManifestPath))
    {
      BOOST_LOG_TRIVIAL(debug) << "appmanifest not found: " << appManifestPath;
      continue;
    }

    std::ifstream acf{appManifestPath};
    auto root = tyti::vdf::read(acf);
    if(root.name != "AppState")
    {
      BOOST_LOG_TRIVIAL(debug) << "Invalid appmanifest";
      continue;
    }

    if(auto it = root.attribs.find("installdir"); it != root.attribs.end())
    {
      const auto fullTestPath = libFolder / "steamapps" / "common" / it->second / testPath;
      if(!std::filesystem::is_regular_file(fullTestPath))
      {
        BOOST_LOG_TRIVIAL(debug) << "File not found: " << fullTestPath;
        continue;
      }

      return fullTestPath;
    }
  }

  return std::nullopt;
}
#endif
} // namespace

bool MainWindow::importBaseGameData()
{
  std::optional<std::filesystem::path> gameDatPath;
  std::optional<std::filesystem::path> tombAtiExePath;
#ifdef WIN32
  gameDatPath = tryGetSteamGamePath("GAME.DAT");
  tombAtiExePath = tryGetSteamGamePath("tombati.exe");
#endif

  if(gameDatPath.has_value())
  {
    QMessageBox askUseFoundImage;
    askUseFoundImage.setWindowTitle(tr("Image Found"));
    askUseFoundImage.setText(tr("Import from found image?"));
    askUseFoundImage.setInformativeText(
      tr("A game data image has been found at %1. Do you want to use this image or continue manually?")
        .arg(gameDatPath->string().c_str()));
    const auto useFoundImageButton = askUseFoundImage.addButton(tr("Use Image"), QMessageBox::AcceptRole);
    askUseFoundImage.addButton(tr("Continue Manually"), QMessageBox::RejectRole);
    askUseFoundImage.setIcon(QMessageBox::Question);

    askUseFoundImage.exec();
    if(askUseFoundImage.clickedButton() == useFoundImageButton)
    {
      extractImage(*gameDatPath, findUserDataDir().value() / "data" / "tr1");
      return true;
    }
  }

  if(tombAtiExePath.has_value())
  {
    QMessageBox askUseFoundExe;
    askUseFoundExe.setWindowTitle(tr("TombATI Found"));
    askUseFoundExe.setText(tr("Import from found TombATI installation?"));
    askUseFoundExe.setInformativeText(
      tr("A TombATI installation has been found at %1. Do you want to use this installation or continue manually?")
        .arg(tombAtiExePath->parent_path().string().c_str()));
    const auto useFoundExeButton = askUseFoundExe.addButton(tr("Use TombATI installation"), QMessageBox::AcceptRole);
    askUseFoundExe.addButton(tr("Continue Manually"), QMessageBox::RejectRole);
    askUseFoundExe.setIcon(QMessageBox::Question);

    askUseFoundExe.exec();
    if(askUseFoundExe.clickedButton() == useFoundExeButton)
    {
      const auto targetDir = findUserDataDir().value() / "data" / "tr1";
      const auto srcPath = QFileInfo{QString::fromUtf8(tombAtiExePath->string().c_str())}.path();
      for(const auto& subDirName : {"FMV", "DATA", "Music"})
      {
        copyDir(srcPath, targetDir, subDirName, true);
      }
      return true;
    }
  }

  const auto imageOrTombExe = QFileDialog::getOpenFileName(
    this, tr("Select Tomb Raider 1 Data"), QString{}, tr("Game Data Files (tomb.exe tombati.exe GAME.DAT)"));
  if(imageOrTombExe.isEmpty())
    return false;

  const auto srcPath = QFileInfo{imageOrTombExe}.path();
  if(QFileInfo{imageOrTombExe}.fileName().toLower() == "game.dat")
  {
    extractImage(imageOrTombExe.toStdString(), findUserDataDir().value() / "data" / "tr1");
  }
  else
  {
    const auto targetDir = findUserDataDir().value() / "data" / "tr1";
    for(const auto& subDirName : {"FMV", "DATA", "Music"})
    {
      copyDir(srcPath, targetDir, subDirName, true);
    }
  }
  return true;
}

void MainWindow::copyDir(const QString& srcPath,
                         const std::filesystem::path& targetDir,
                         const std::string& subDirName,
                         bool overwriteExisting)
{
  std::filesystem::create_directories(targetDir / subDirName);

  const auto srcSubPath = srcPath + QDir::separator() + subDirName.c_str();
  for(const auto& fileName : QDir{srcSubPath}.entryList(QDir::Files))
  {
    const auto srcFilename = srcSubPath + QDir::separator() + fileName;
    const auto dstFilename = QString((targetDir / subDirName).string().c_str()) + QDir::separator() + fileName;
    if(!overwriteExisting && QFileInfo::exists(dstFilename))
    {
      BOOST_LOG_TRIVIAL(info) << "Copy " << srcFilename.toStdString() << " to " << dstFilename.toStdString()
                              << " skipped (already exists)";
      continue;
    }

    if(QFile::exists(dstFilename))
    {
      BOOST_LOG_TRIVIAL(info) << "Delete " << dstFilename.toStdString();
      if(!QFile::remove(dstFilename))
      {
        QMessageBox::critical(this, tr("Copy Failed"), tr("Failed to delete %1").arg(dstFilename));
        return;
      }
    }

    BOOST_LOG_TRIVIAL(info) << "Copy " << srcFilename.toStdString() << " to " << dstFilename.toStdString();
    if(!QFile::copy(srcFilename, dstFilename))
    {
      QMessageBox::critical(this, tr("Copy Failed"), tr("Failed to copy %1 to %2").arg(srcFilename, dstFilename));
      return;
    }
  }
}

void MainWindow::onMigrateClicked()
{
  const auto fileName
    = QFileDialog::getOpenFileName(this,
                                   tr("Select Other CroftEngine/EdisonEngine Installation"),
                                   QString{},
                                   tr("CroftEngine/EdisonEngine files (croftengine.exe edisonengine.exe config.yaml)"));
  if(fileName.isEmpty())
    return;

  QMessageBox askDataOverwrite;
  askDataOverwrite.setWindowTitle(tr("Data Migration"));
  askDataOverwrite.setText(tr("Overwrite Existing Data?"));
  askDataOverwrite.setInformativeText(
    tr("Decide to keep already existing ghosts, savegames, etc. in this installation. If you decide to overwrite "
       "already existing files, the data will be lost."));
  const auto overwriteBtn = askDataOverwrite.addButton(tr("Overwrite"), QMessageBox::ButtonRole::YesRole);
  const auto keepBtn = askDataOverwrite.addButton(tr("Don't overwrite"), QMessageBox::ButtonRole::NoRole);
  /*const auto abortBtn = */ askDataOverwrite.addButton("Abort", QMessageBox::ButtonRole::RejectRole);
  askDataOverwrite.setDefaultButton(QMessageBox::StandardButton::No);
  askDataOverwrite.setIcon(QMessageBox::Icon::Question);

  askDataOverwrite.exec();
  bool overwrite;
  if(askDataOverwrite.clickedButton() == overwriteBtn)
  {
    overwrite = true;
  }
  else if(askDataOverwrite.clickedButton() == keepBtn)
  {
    overwrite = false;
  }
  else
  {
    return;
  }

  const auto userDataDir = findUserDataDir().value();
  for(const auto& subDir : {"saves", "ghosts", "screenshots"})
    copyDir(QFileInfo{fileName}.path(), userDataDir, subDir, overwrite);

  const auto newConfig = userDataDir / "config.yaml";
  if(overwrite || !std::filesystem::is_regular_file(newConfig))
  {
    if(std::filesystem::is_regular_file(newConfig))
      std::filesystem::remove(newConfig);
    if(const auto oldConfig = QFileInfo{fileName}.path() + QDir::separator() + "config.yaml";
       !QFile::copy(oldConfig, newConfig.string().c_str()))
    {
      QMessageBox::critical(
        this, tr("Copy Failed"), tr("Failed to copy %1 to %2").arg(oldConfig, newConfig.string().c_str()));
      return;
    }
  }

  QMessageBox::information(this, tr("Data Migrated"), tr("Your old data has been migrated."));
}

void MainWindow::extractSoundtrackZip(std::filesystem::path target)
{
  for(const auto gameflow : {"tr1", "tr1demo-part1", "tr1demo-part2"})
  {
    const auto gameflowRoot = findUserDataDir().value() / "data" / gameflow;
    if(!std::filesystem::is_directory(gameflowRoot))
    {
      continue;
    }

    ReadOnlyArchive archive{target};
    if(archive.failure())
    {
      QMessageBox::critical(
        this,
        tr("Extraction Error"),
        tr("Could not open %1 as an archive: %2")
          .arg(target.string().c_str(), archive.getErrorString().value_or("Unknown error").c_str()));
      return;
    }

    const auto dataRoot = gameflowRoot / "AUDIO";
    while(archive.next())
    {
      const auto dstName = dataRoot / archive.getCurrentPathName();
      std::filesystem::create_directories(dstName.parent_path());
      archive.writeCurrentTo(dstName);
    }
  }

  QMessageBox::information(this, tr("Soundtrack Downloaded"), tr("The Soundtrack has been downloaded successfully."));
}

void MainWindow::resetConfig()
{
  const auto configPath = findUserDataDir().value() / "config.yaml";
  QFile::remove(configPath.string().c_str());
}

void MainWindow::onSelectGlidosClicked()
{
  const auto userDataPath = findUserDataDir();
  if(!userDataPath.has_value() || !std::filesystem::is_regular_file(*userDataPath / "config.yaml"))
  {
    QMessageBox::warning(
      this, tr("Not Configured"), tr("To be able to configure a texture pack, you need to start the engine once."));
    return;
  }

  {
    QMessageBox askPackType;
    askPackType.setWindowTitle(tr("Texture Pack Type"));
    askPackType.setText(tr("Please select Texture Pack Type"));
    askPackType.setInformativeText(
      tr("Please select what type of texture pack you want to activate. If your texture pack contains a equiv.txt "
         "file, use the first option. If your texture pack contains a series of folders which are made of 32 numbers "
         "and letters, use the second one."));
    const auto useEquiv = askPackType.addButton("equiv.txt", QMessageBox::ButtonRole::AcceptRole);
    /*const auto useFolders = */ askPackType.addButton(tr("Folders"), QMessageBox::ButtonRole::AcceptRole);
    askPackType.setIcon(QMessageBox::Icon::Question);
    askPackType.exec();
    if(askPackType.clickedButton() == useEquiv)
    {
      QMessageBox::information(
        this,
        tr("Texture Pack Main File"),
        tr("In the following dialog, select a file from the top-most directory of the texture pack."));
      const auto texturePack = QFileDialog::getOpenFileName(
        this, tr("Select Glidos Texture Pack Main File"), QString{}, tr("Texture Pack Main File (equiv.txt)"));
      if(texturePack.isEmpty())
        return;

      const QFileInfo info{texturePack};
      setGlidosPath(info.absolutePath().toStdString());
    }
    else
    {
      const auto texturePack = QFileDialog::getExistingDirectory(this, tr("Select Glidos Texture Pack Main File"));
      if(texturePack.isEmpty())
        return;

      const QFileInfo info{texturePack};
      setGlidosPath(info.absoluteFilePath().toStdString());
    }
  }
}

void MainWindow::setGlidosPath(const std::optional<std::string>& path)
{
  const auto userDataPath = findUserDataDir();

  std::string oldLocale = gsl::not_null{setlocale(LC_NUMERIC, nullptr)}.get();
  setlocale(LC_NUMERIC, "C");

  std::string buffer;
  {
    std::ifstream file{*userDataPath / "config.yaml", std::ios::in};
    gsl_Assert(file.is_open());
    file.seekg(0, std::ios::end);
    const auto size = static_cast<std::size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    buffer.resize(size);
    file.read(buffer.data(), gsl::narrow<std::streamsize>(size));
  }

  setlocale(LC_NUMERIC, oldLocale.c_str());

  auto tree = ryml::parse_in_arena(c4::to_csubstr((*userDataPath / "config.yaml").string()), c4::to_csubstr(buffer));
  auto root = tree.rootref();
  if(!root["config"].is_map() || !root["config"]["renderSettings"].is_map())
  {
    QMessageBox::critical(
      this, tr("Invalid Config"), tr("Your configuration file is invalid. Reset your configuration."));
    return;
  }

  {
    auto node = root["config"]["renderSettings"];
    node.remove_child("glidosPack");
    auto glidosPack = node[tree.copy_to_arena(c4::to_csubstr("glidosPack"))];
    if(path.has_value())
    {
      glidosPack << *path;
    }
    else
    {
      glidosPack << "~";
      glidosPack.set_val_tag(tree.copy_to_arena(c4::to_csubstr("!!null")));
    }
  }

  oldLocale = gsl::not_null{setlocale(LC_NUMERIC, nullptr)}.get();
  setlocale(LC_NUMERIC, "C");

  {
    std::ofstream file{*userDataPath / "config.yaml", std::ios::out | std::ios::trunc};
    gsl_Assert(file.is_open());
    file << tree.rootref();
  }

  setlocale(LC_NUMERIC, oldLocale.c_str());
}

void MainWindow::onDisableGlidosClicked()
{
  setGlidosPath(std::nullopt);
}

void MainWindow::onLaunchClicked()
{
  QSettings settings;
  const auto languageId = m_languages.data(m_languages.index(ui->languages->currentIndex(), 0), IdRole).toString();
  settings.setValue(LanguageConfigKey, QVariant{languageId});
  const auto gameflowId = ui->gameflows->currentIndex().data(IdRole).toString();
  settings.setValue(GameflowConfigKey, QVariant{gameflowId});
  const auto languageIdData = languageId.toUtf8();
  const auto gameflowIdData = gameflowId.toUtf8();
  m_launchRequest = std::tuple<std::string, std::string>{
    std::string{languageIdData.data(), gsl::narrow<size_t>(languageIdData.size())},
    std::string{gameflowIdData.data(), gsl::narrow<size_t>(gameflowIdData.size())}};

  NetworkConfig cfg{};
  cfg.color = {gsl::narrow_cast<uint8_t>(m_ghostColor.red()),
               gsl::narrow_cast<uint8_t>(m_ghostColor.green()),
               gsl::narrow_cast<uint8_t>(m_ghostColor.blue())};
  cfg.socket = ui->serverSocket->text().toStdString();
  cfg.username = ui->username->text().toStdString();
  cfg.authToken = ui->authToken->text().toStdString();
  cfg.sessionId = ui->sessionId->text().toStdString();
  cfg.save();

  close();
}

void MainWindow::onGameflowSelected(const QModelIndex& index)
{
  while(!ui->gameflowMeta->isEmpty())
  {
    const auto child = ui->gameflowMeta->takeAt(0);
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    delete child->widget();
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    delete child;
  }

  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
  m_importButton = new QPushButton(tr("Import Data"), this);
  QObject::connect(m_importButton, &QPushButton::clicked, this, &MainWindow::onImportClicked);

  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
  ui->gameflowMeta->addWidget(new QLabel(tr("By %1").arg(m_gameflows.data(index, AuthorRole).toString())));
  ui->gameflowMeta->setStretch(0, 0);
  for(const auto& url : m_gameflows.data(index, UrlsRole).toStringList())
  {
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto lbl = new QLabel();
    lbl->setText(QString("<a href=\"%1\">%1</a>").arg(url));
    lbl->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
    lbl->setOpenExternalLinks(true);

    ui->gameflowMeta->addWidget(lbl);
    ui->gameflowMeta->setStretchFactor(lbl, 0);
  }

  ui->gameflowMeta->addStretch(1);
  ui->gameflowMeta->addWidget(m_importButton);
}

void MainWindow::onChooseColorClicked()
{
  if(const auto newColor = QColorDialog::getColor(m_ghostColor, this, tr("Choose Your Ghost Color"));
     newColor.isValid())
    m_ghostColor = newColor;

  QPalette pal;
  pal.setColor(QPalette::Window, m_ghostColor);
  ui->lblColor->setPalette(pal);
}

void MainWindow::onTestConnectionClicked()
{
  const QUrl url{"coop://" + ui->serverSocket->text()};
  if(!url.isValid() || url.hasFragment() || url.hasQuery() || !url.path().isEmpty() || url.port() == -1
     || url.scheme() != "coop" || !url.userInfo().isEmpty())
  {
    QMessageBox::warning(this,
                         tr("Invalid Connection Settings"),
                         tr("Ensure you enter your server address only containing the domain name and the port, for "
                            "example like 'example.com:12345'."));
    return;
  }

  if(ui->username->text().isEmpty())
  {
    QMessageBox::warning(this, tr("Invalid Connection Settings"), tr("Please fill in your username."));
    return;
  }

  if(!QRegExp{"[a-f0-9]{32}"}.exactMatch(ui->authToken->text()))
  {
    QMessageBox::warning(
      this,
      tr("Invalid Connection Settings"),
      tr("Invalid auth token, make sure you copied it from the website. This is NOT your password."));
    return;
  }

  if(!QRegExp{"[a-f0-9]{32}"}.exactMatch(ui->sessionId->text()))
  {
    QMessageBox::warning(
      this, tr("Invalid Connection Settings"), tr("Invalid session ID, make sure you copied it from the website."));
    return;
  }

  QMessageBox::information(this,
                           tr("Valid Connection Settings"),
                           tr("Your configuration seems valid. However, no attempt was made to check against the "
                              "server that it will actually work."));
}

void MainWindow::populateCrashdumpsList()
{
  auto userDataDir = findUserDataDir();
  if(!userDataDir.has_value())
    return;

  auto dumpDir = userDataDir.value() / "crashdumps" / CE_GIT_TAG;

  for(const auto& file : std::filesystem::directory_iterator::directory_iterator(dumpDir))
  {
    QList<QStandardItem*> item;
    item.append(new QStandardItem(QString::fromLatin1(file.path().filename().string().c_str())));
    // https://developercommunity.visualstudio.com/t/stdfilesystemfile-time-type-does-not-allow-easy-co/251213
    time_t lastWriteTime
      = std::chrono::duration_cast<std::chrono::seconds>(file.last_write_time().time_since_epoch() - 11644473600s)
          .count();
    item.append(new QStandardItem(QString::fromLatin1(ctime(&lastWriteTime))));
    m_crashdumps.appendRow(item);
  }

  if(m_crashdumps.rowCount() > 0)
  {
    ui->tabWidget->setCurrentIndex(3);
  }

  m_crashdumps.setHorizontalHeaderLabels({"Filename", "Date"});
  ui->crashdumps->resizeColumnsToContents();
}

void MainWindow::onCleanUpClicked()
{
  QMessageBox msgbox;
  msgbox.setText("This is not implemented yet");
  msgbox.addButton(QMessageBox::Ok);
  msgbox.exec();
}

void MainWindow::onOpenCrashdumpsClicked()
{
  QDesktopServices::openUrl("file:///" + QString::fromUtf8( (findUserDataDir().value() / "crashdumps" / CE_GIT_TAG).string().c_str()));
}

} // namespace launcher
