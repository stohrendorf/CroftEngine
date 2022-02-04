#include "setup.h"

#include "mainwindow.h"

#include <QApplication>

namespace setup
{
void showSetupScreen(int argc, char** argv)
{
#ifdef WIN32
  putenv(("QT_PLUGIN_PATH=" + std::filesystem::current_path().string() + "/plugins").c_str());
#endif

  QApplication app{argc, argv};
  MainWindow w;
  w.show();
  QApplication::exec();
}
} // namespace setup
