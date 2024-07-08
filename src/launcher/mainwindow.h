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
// NOLINTNEXTLINE(*-include-cleaner)
class MainWindow : public QMainWindow
{
  // NOLINTNEXTLINE(*-include-cleaner)
  Q_OBJECT

public:
  // NOLINTNEXTLINE(*-include-cleaner)
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow() override;

  [[nodiscard]] const auto& getLaunchRequest() const
  {
    return m_launchRequest;
  }

  // NOLINTNEXTLINE(*-include-cleaner)
private slots:
  void onOpenDataLocationClicked();
  void onMigrateClicked();
  void onImportClicked();
  void onSelectGlidosClicked();
  void onDisableGlidosClicked();
  void extractSoundtrackZip(const std::filesystem::path& target);
  void resetConfig();
  void onLaunchClicked();
  // NOLINTNEXTLINE(*-include-cleaner)
  void onGameflowSelected(const QModelIndex& index);
  void onChooseColorClicked();
  void onTestConnectionClicked();
  // NOLINTNEXTLINE(*-include-cleaner)
  void downloadedReleasesJson(QNetworkReply* reply);

private:
  Ui::MainWindow* ui;
  bool importBaseGameData();
  // NOLINTNEXTLINE(*-include-cleaner)
  void copyDir(const QString& srcPath,
               const std::filesystem::path& targetDir,
               const std::string& subDirName,
               bool overwriteExisting);

  void setGlidosPath(const std::optional<std::string>& path);
  void updateUpdateBar();

  // NOLINTNEXTLINE(*-include-cleaner)
  QStandardItemModel m_languages;
  QStandardItemModel m_gameflows;
  std::optional<std::tuple<std::string, std::string, bool>> m_launchRequest = std::nullopt;
  // NOLINTNEXTLINE(*-include-cleaner)
  QPushButton* m_importButton = nullptr;
  // NOLINTNEXTLINE(*-include-cleaner)
  QColor m_ghostColor;
  // NOLINTNEXTLINE(*-include-cleaner)
  QNetworkAccessManager m_releasesNetworkAccessManager;
};
} // namespace launcher
