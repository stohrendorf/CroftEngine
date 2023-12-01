#include "launcher.h"

#include "mainwindow.h"
#include "paths.h"

#include <gsl/gsl-lite.hpp>
#include <QApplication>
#include <QDebug>
#include <QTranslator>

namespace launcher
{
std::optional<std::tuple<std::string, std::string,bool>> showLauncher(int argc, char** argv)
{
#ifdef WIN32
  gsl_Assert(_putenv(("QT_PLUGIN_PATH=" + std::filesystem::current_path().string() + "/plugins").c_str()) == 0);
#endif

  QCoreApplication::setOrganizationName("stohrendorf");
  QCoreApplication::setOrganizationDomain("earvillage.net");
  QCoreApplication::setApplicationName("CroftEngine");

  const QApplication app{argc, argv};

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
  return w.getLaunchRequest();
}
} // namespace launcher
