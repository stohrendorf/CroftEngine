// --- FIXME: for whatever reason, compilation fails if this isn't included before mainwindow.h
#include <ryml.hpp>
#include <ryml_std.hpp>
// ---
#include "gameflow/meta.h"
#include "languages.h"
#include "mainwindow.h"
#include "serialization/serialization.h"
#include "serialization/yamldocument.h"
// ---

#include "cdrom.h"
#include "downloadprogress.h"
#include "mscdex.h"
#include "paths.h"
#include "ui_mainwindow.h"

#include <archive.h>
#include <archive_entry.h>
#include <boost/throw_exception.hpp>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>

#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN
#  include "vdf_parser.hpp"

#  include <windows.h>
#endif

namespace launcher
{
namespace
{
const char* const LanguageConfigKey = "launcher-language";
const char* const GameflowConfigKey = "launcher-gameflow";
const int IdRole = Qt::UserRole + 1;

void extractImage(const std::filesystem::path& cueFile, const std::filesystem::path& targetDir)
{
  auto img = std::make_unique<cdrom::CdImage>(cueFile);
  for(const auto& [path, span] : cdrom::getFiles(*img))
  {
    gsl_Assert(!path.empty());
    const auto root = *path.begin();
    if(root == "DATA" || root == "FMV")
    {
      qInfo() << QString("Extracting %1 to %2 from %3")
                   .arg(path.string().c_str(), (targetDir / path).string().c_str(), cueFile.string().c_str());
      std::error_code ec;
      std::filesystem::create_directories(targetDir / path.parent_path(), ec);
      const auto data = cdrom::readFile(*img, span);
      std::ofstream tmp{targetDir / path, std::ios::binary | std::ios::trunc};
      tmp.write((const char*)data.data(), data.size());
    }
  }
}

#ifdef WIN32
std::optional<std::filesystem::path> readRegistryPath(HKEY hKey, const std::wstring& path, const std::wstring& key)
{
  HKEY subKey;
  if(RegOpenKeyExW(hKey, path.c_str(), 0, KEY_READ, &subKey) != 0)
  {
    RegCloseKey(subKey);
    return std::nullopt;
  }

  WCHAR szBuffer[512];
  DWORD dwBufferSize = sizeof(szBuffer);
  DWORD type = REG_NONE;
  const auto nError = RegQueryValueExW(subKey, key.c_str(), nullptr, &type, (LPBYTE)szBuffer, &dwBufferSize);
  RegCloseKey(subKey);

  if(nError != 0 || type != REG_SZ)
  {
    return std::nullopt;
  }
  return std::filesystem::path{szBuffer};
}
#endif
} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  QSettings settings;

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
        ui->languages->setCurrentIndex(m_languages.index(i, 0));
      ++i;
    }
  }

  {
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
      doc.load("meta", meta, meta);
      metas.emplace_back(it->path().stem().string(), meta);
    }

    m_gameflows.insertRows(0, gsl::narrow<int>(metas.size()));
    int i = 0;
    for(const auto& [gameflowId, gameflowMeta] : metas)
    {
      auto item = new QStandardItem{QString::fromLatin1(gameflowMeta.title.c_str())};
      item->setData(QVariant{QString::fromLatin1(gameflowId.c_str())}, IdRole);
      m_gameflows.setItem(i, item);

      if(gameflow == QString::fromLatin1(gameflowId.c_str()))
        ui->gameflows->setCurrentIndex(m_gameflows.index(i, 0));
      ++i;
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

  QObject::connect(ui->openDataLocation, &QPushButton::clicked, this, &MainWindow::onOpenDataLocationClicked);
  QObject::connect(ui->migrateBtn, &QPushButton::clicked, this, &MainWindow::onMigrateClicked);
  QObject::connect(ui->importBtn, &QPushButton::clicked, this, &MainWindow::onImportClicked);
  QObject::connect(ui->resetConfig, &QPushButton::clicked, this, &MainWindow::resetConfig);
  QObject::connect(ui->selectGlidos, &QPushButton::clicked, this, &MainWindow::onSelectGlidosClicked);
  QObject::connect(ui->disableGlidos, &QPushButton::clicked, this, &MainWindow::onDisableGlidosClicked);
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
  if(!importGameData())
    return;

  QMessageBox::information(this, tr("Data Imported"), tr("Game Data has been imported."));

  if(std::filesystem::is_regular_file(findUserDataDir().value() / "data" / "tr1" / "AUDIO" / "002.ogg")
     || std::filesystem::is_regular_file(findUserDataDir().value() / "data" / "tr1" / "Music" / "Track02.flac"))
    return;

  auto downloader = new DownloadProgress(QUrl{"https://opentomb.earvillage.net/croftengine-audio-tr1.zip"},
                                         findUserDataDir().value() / "data" / "tr1" / "AUDIO" / "tracks.zip",
                                         this);
  connect(downloader, &DownloadProgress::downloaded, this, &MainWindow::extractSoundtrackZip);
  downloader->show();
  downloader->start();
}

namespace
{
#ifdef WIN32
std::optional<std::filesystem::path> tryGetSteamGamePath(const std::filesystem::path& testPath)
{
  const auto tryGetLibraryFolders = [](const std::wstring& path) -> std::vector<std::filesystem::path>
  {
    const auto installPath = readRegistryPath(HKEY_LOCAL_MACHINE, path, L"InstallPath");
    if(!installPath.has_value())
    {
      qDebug() << "Steam InstallPath not found in registry";
      return {};
    }

    const auto libraryFolderVdfPath = *installPath / "steamapps" / "libraryfolders.vdf";
    if(!std::filesystem::is_regular_file(libraryFolderVdfPath))
    {
      qDebug() << "libraryfolders.vdf not found";
      return {};
    }

    std::ifstream vdf{libraryFolderVdfPath};
    auto root = tyti::vdf::read(vdf);
    if(root.name != "libraryfolders")
    {
      qDebug() << "Invalid libraryfolders.vdf";
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
        qDebug() << "Invalid library folder entry key";
        continue;
      }

      if(auto it = entryContent->attribs.find("path"); it != entryContent->attribs.end())
      {
        qDebug() << "Found library folder" << it->second.c_str();
        paths.emplace_back(it->second);
      }
      else
      {
        qDebug() << "Incomplete libraryfolders entry content";
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
    qDebug() << "Check manifest:" << appManifestPath.string().c_str();
    if(!std::filesystem::is_regular_file(appManifestPath))
    {
      qDebug() << "appmanifest not found:" << appManifestPath.string().c_str();
      continue;
    }

    std::ifstream acf{appManifestPath};
    auto root = tyti::vdf::read(acf);
    if(root.name != "AppState")
    {
      qDebug() << "Invalid appmanifest";
      continue;
    }

    if(auto it = root.attribs.find("installdir"); it != root.attribs.end())
    {
      const auto fullTestPath = libFolder / "steamapps" / "common" / it->second / testPath;
      if(!std::filesystem::is_regular_file(fullTestPath))
      {
        qDebug() << "File not found:" << fullTestPath.string().c_str();
        continue;
      }

      return fullTestPath;
    }
  }

  return std::nullopt;
}
#endif
} // namespace

bool MainWindow::importGameData()
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
    askUseFoundImage.addButton("Continue Manually", QMessageBox::RejectRole);
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
  std::error_code ec;
  std::filesystem::create_directories(targetDir / subDirName, ec);

  const auto srcSubPath = srcPath + QDir::separator() + subDirName.c_str();
  for(const auto& fileName : QDir{srcSubPath}.entryList(QDir::Files))
  {
    const auto srcFilename = srcSubPath + QDir::separator() + fileName;
    const auto dstFilename = QString((targetDir / subDirName).string().c_str()) + QDir::separator() + fileName;
    if(!overwriteExisting && QFileInfo::exists(dstFilename))
    {
      qInfo() << QString("Copy %1 to %2 skipped (already exists)").arg(srcFilename, dstFilename);
      continue;
    }

    if(QFile::exists(dstFilename))
    {
      qInfo() << QString("Delete %1").arg(dstFilename);
      if(!QFile::remove(dstFilename))
      {
        QMessageBox::critical(this, tr("Copy Failed"), tr("Failed to delete %1").arg(dstFilename));
        return;
      }
    }

    qInfo() << QString("Copy %1 to %2").arg(srcFilename, dstFilename);
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
  const auto a = archive_read_new();
  archive_read_support_format_all(a);
  archive_read_support_compression_all(a);
  if(auto r = archive_read_open_filename(a, target.string().c_str(), 10240); r != ARCHIVE_OK)
  {
    QMessageBox::critical(
      this,
      tr("Extraction Error"),
      tr("Could not open %1 as an archive: %2").arg(target.string().c_str(), archive_error_string(a)));
    return;
  }

  std::vector<char> buffer;
  buffer.resize(8192);

  const auto dataRoot = findUserDataDir().value() / "data" / "tr1" / "AUDIO";

  while(true)
  {
    archive_entry* entry = nullptr;
    auto r = archive_read_next_header(a, &entry);
    if(r == ARCHIVE_EOF)
      break;

    if(r != ARCHIVE_OK)
    {
      qWarning() << archive_error_string(a);
    }
    if(r < ARCHIVE_WARN)
    {
      QMessageBox::critical(
        this,
        tr("Extraction Error"),
        tr("Could not extract from archive %1: %2").arg(target.string().c_str(), archive_error_string(a)));
      return;
    }
    const auto dstName = dataRoot / archive_entry_pathname(entry);
    {
      std::filesystem::create_directories(dstName.parent_path());
    }
    std::ofstream dst{dstName, std::ios::binary | std::ios::trunc};
    while(true)
    {
      const auto read = archive_read_data(a, buffer.data(), buffer.size());
      if(read == 0)
        break;
      dst.write(buffer.data(), read);
    }
  }
  archive_read_close(a);
  archive_read_free(a);

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
    Expects(file.is_open());
    file.seekg(0, std::ios::end);
    const auto size = static_cast<std::size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    buffer.resize(size);
    file.read(&buffer[0], size);
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
    Expects(file.is_open());
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
  const auto languageId = ui->languages->currentIndex().data(IdRole).toString();
  qInfo() << "language id" << languageId;
  settings.setValue(LanguageConfigKey, QVariant{languageId});
  const auto gameflowId = ui->gameflows->currentIndex().data(IdRole).toString();
  qInfo() << "gameflow id" << gameflowId;
  settings.setValue(GameflowConfigKey, QVariant{gameflowId});
  const auto languageIdData = languageId.toUtf8();
  const auto gameflowIdData = gameflowId.toUtf8();
  m_launchRequest = std::tuple<std::string, std::string>{
    std::string{languageIdData.data(), gsl::narrow<size_t>(languageIdData.size())},
    std::string{gameflowIdData.data(), gsl::narrow<size_t>(gameflowIdData.size())}};
  close();
}
} // namespace launcher
