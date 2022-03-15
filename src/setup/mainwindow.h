#pragma once

#include <QMainWindow>
// https://bugreports.qt.io/browse/QTBUG-73263
#include <filesystem>
#include <optional>

namespace Ui
{
class MainWindow;
}

namespace setup
{
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow() override;

private slots:
  void onOpenDataLocationClicked();
  void onMigrateClicked();
  void onImportClicked();
  void onSelectGlidosClicked();
  void onDisableGlidosClicked();
  void extractSoundtrackZip(std::filesystem::path target);
  void resetConfig();

private:
  Ui::MainWindow* ui;
  bool importGameData();
  void copyDir(const QString& srcPath,
               const std::filesystem::path& targetDir,
               const std::string& subDirName,
               bool overwriteExisting);

  void setGlidosPath(const std::optional<std::string>& path);
};
} // namespace setup
