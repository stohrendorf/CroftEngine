#include "setup.h"

#include "mainwindow.h"
#include "paths.h"

#include <QApplication>
#include <QDebug>
#include <QTranslator>

namespace setup
{
void showSetupScreen(int argc, char** argv)
{
#ifdef WIN32
  putenv(("QT_PLUGIN_PATH=" + std::filesystem::current_path().string() + "/plugins").c_str());
#endif

  QApplication app{argc, argv};

  QTranslator translator;
  if(!translator.load(QLocale(),
                      QLatin1String("croftengine"),
                      QLatin1String("_"),
                      QString::fromLatin1((findEngineDataDir().value() / "i18n").string().c_str()),
                      QLatin1String(".qm")))
  {
    qWarning() << "failed to load translations for" << QLocale() << "/" << QLocale().name();
  }
  QApplication::installTranslator(&translator);

  MainWindow w;
  w.show();
  QApplication::exec();
}
} // namespace setup
