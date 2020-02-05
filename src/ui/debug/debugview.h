#pragma once

#include <QApplication>
#include <QTabWidget>
#include <QTableWidget>
#include <QWidget>
#include <gsl-lite.hpp>
#include <map>
#include <set>

namespace engine::objects
{
class Object;
class LaraObject;
} // namespace engine::objects

namespace ui::debug
{
class LaraInfoWidget;

class DebugView
{
public:
  explicit DebugView(int argc, char** argv);
  ~DebugView();

  static void processEvents()
  {
    QApplication::processEvents(QEventLoop::ProcessEventsFlag::AllEvents, 5);
  }

  [[nodiscard]] bool isVisible() const
  {
    return m_window.isVisible();
  }

  void toggleVisibility()
  {
    if(isVisible())
      m_window.hide();
    else
      m_window.show();
  }

  void update(const engine::objects::LaraObject& lara,
              const std::map<uint16_t, gsl::not_null<std::shared_ptr<engine::objects::Object>>>& objects,
              const std::set<gsl::not_null<std::shared_ptr<engine::objects::Object>>>& dynamicObjects);

private:
  QApplication m_application;
  QWidget m_window;
  QTabWidget* m_tabs = new QTabWidget();
  LaraInfoWidget* m_laraInfo;
  QTableWidget* m_triggerTable = new QTableWidget();
};
} // namespace ui::debug
