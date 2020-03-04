#pragma once

#include <QApplication>
#include <QTabWidget>
#include <QTableWidget>
#include <QWidget>
#include <gsl-lite.hpp>
#include <map>
#include <mutex>
#include <set>
#include <thread>

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

  [[nodiscard]] bool isVisible() const
  {
    std::lock_guard<std::recursive_mutex> guard{m_mutex};
    return m_window.isVisible();
  }

  void toggleVisibility()
  {
    std::lock_guard<std::recursive_mutex> guard{m_mutex};
    if(isVisible())
      m_window.hide();
    else
      m_window.show();
  }

  void update(const engine::objects::LaraObject& lara,
              const std::map<uint16_t, gsl::not_null<std::shared_ptr<engine::objects::Object>>>& objects,
              const std::set<gsl::not_null<std::shared_ptr<engine::objects::Object>>>& dynamicObjects);

  void stop()
  {
    if(m_stop)
      return;

    m_stop = true;
    m_thread.join();
  }

private:
  QApplication m_application;
  QWidget m_window;
  QTabWidget* m_tabs = new QTabWidget();
  LaraInfoWidget* m_laraInfo;
  QTableWidget* m_triggerTable = new QTableWidget();
  std::thread m_thread;
  mutable std::recursive_mutex m_mutex;
  mutable bool m_stop = false;

  void processEvents() const
  {
    while(!m_stop)
    {
      QApplication::processEvents(QEventLoop::ProcessEventsFlag::AllEvents, 5);
      std::this_thread::sleep_for(std::chrono::milliseconds{5});
      std::this_thread::yield();
    }
  }
};
} // namespace ui::debug
