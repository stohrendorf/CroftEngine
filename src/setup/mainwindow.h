#pragma once

#include <QMainWindow>
// https://bugreports.qt.io/browse/QTBUG-73263
#include <filesystem>

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
  void extractSoundtrackZip(std::filesystem::path target);
  void resetConfig();

private:
  Ui::MainWindow* ui;
  bool importGameData();
  void copyDir(const QString& srcPath,
               const std::filesystem::path& targetDir,
               const std::string& subDirName,
               bool overwriteExisting);
};
} // namespace setup
