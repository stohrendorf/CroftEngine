#pragma once

#include <QMainWindow>
#include <QNetworkReply>
#include <QPushButton>
#include <QStandardItemModel>
// https://bugreports.qt.io/browse/QTBUG-73263
#include <filesystem>
#include <optional>
#include <string>
#include <tuple>

namespace Ui
{
class MainWindow;
}

namespace launcher
{
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow() override;

  [[nodiscard]] const auto& getLaunchRequest() const
  {
    return m_launchRequest;
  }

private slots:
  void onOpenDataLocationClicked();
  void onMigrateClicked();
  void onImportClicked();
  void onSelectGlidosClicked();
  void onDisableGlidosClicked();
  void extractSoundtrackZip(std::filesystem::path target);
  void resetConfig();
  void onLaunchClicked();
  void onGameflowSelected(const QModelIndex& index);
  void onChooseColorClicked();
  void onTestConnectionClicked();
  void downloadedReleasesJson(QNetworkReply* reply);

private:
  Ui::MainWindow* ui;
  bool importBaseGameData();
  void copyDir(const QString& srcPath,
               const std::filesystem::path& targetDir,
               const std::string& subDirName,
               bool overwriteExisting);

  void setGlidosPath(const std::optional<std::string>& path);
  void updateUpdateBar();

  std::optional<std::filesystem::path> findSuspectArchiveDataRoot(const QString& archiveFilePath);
  void extractArchive(const QString& archiveFilePath,
                      const std::filesystem::path& archiveDataRoot,
                      const std::string& gameflowId);
  void downloadSoundtrackIfNecessary(const std::string& gameflowId);
  ;

  QStandardItemModel m_languages{};
  QStandardItemModel m_gameflows{};
  std::optional<std::tuple<std::string, std::string,bool>> m_launchRequest = std::nullopt;
  QPushButton* m_importButton = nullptr;
  QColor m_ghostColor;
  QNetworkAccessManager m_releasesNetworkAccessManager;
};
} // namespace launcher
