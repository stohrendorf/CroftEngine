#pragma once

#include <QMainWindow>
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

  const auto& getLaunchRequest() const
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

private:
  Ui::MainWindow* ui;
  bool importBaseGameData();
  void copyDir(const QString& srcPath,
               const std::filesystem::path& targetDir,
               const std::string& subDirName,
               bool overwriteExisting);

  void setGlidosPath(const std::optional<std::string>& path);

  QStandardItemModel m_languages{};
  QStandardItemModel m_gameflows{};
  std::optional<std::tuple<std::string, std::string>> m_launchRequest = std::nullopt;
  QPushButton* m_importButton = nullptr;
  QColor m_ghostColor;
};
} // namespace launcher
