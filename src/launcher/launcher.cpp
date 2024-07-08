#include "launcher.h"

#include "mainwindow.h"
#include "paths.h"

#include <filesystem>
#include <gsl/gsl-lite.hpp>
#include <optional>
#include <QApplication>
#include <QDebug>
#include <QTranslator>
#include <string>
#include <tuple>

namespace launcher
{
std::optional<std::tuple<std::string, std::string, bool>> showLauncher(int argc, char** argv)
{
#ifdef WIN32
  gsl_Assert(_putenv(("QT_PLUGIN_PATH=" + std::filesystem::current_path().string() + "/plugins").c_str()) == 0);
#endif

  QCoreApplication::setOrganizationName("stohrendorf");
  QCoreApplication::setOrganizationDomain("earvillage.net");
  QCoreApplication::setApplicationName("CroftEngine");

  const QApplication app{argc, argv};

  const auto engineDataDir = findEngineDataDir();
  gsl_Assert(engineDataDir.has_value());

  QTranslator translator;
  if(!translator.load(QLocale(),
                      QLatin1String("croftengine"),
                      QLatin1String("_"),
                      // NOLINTNEXTLINE(*-unchecked-optional-access)
                      QString::fromLatin1((engineDataDir.value() / "i18n").string().c_str()),
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
