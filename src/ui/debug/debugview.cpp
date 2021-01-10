#include "debugview.h"

#include "larainfowidget.h"

#include <QVBoxLayout>

namespace ui::debug
{
DebugView::DebugView(int argc, char** argv)
    : m_application{argc, argv}
    , m_laraInfo{new LaraInfoWidget()}
    , m_thread{&DebugView::processEvents, this}
{
  m_window.resize(640, 480);
  m_window.show();
  m_window.setWindowTitle("Debug View");

  m_window.setLayout(new QVBoxLayout());
  m_window.layout()->addWidget(m_tabs);

  auto tabWidget = new QWidget();
  tabWidget->setLayout(new QVBoxLayout());

  m_tabs->addTab(m_laraInfo, "Lara");

  m_tabs->addTab(m_triggerTable, "Triggers");
  m_triggerTable->setColumnCount(3);
}

DebugView::~DebugView()
{
  stop();
}

void DebugView::update(const engine::objects::LaraObject& lara,
                       const std::map<uint16_t, gsl::not_null<std::shared_ptr<engine::objects::Object>>>& objects,
                       const std::set<gsl::not_null<std::shared_ptr<engine::objects::Object>>>& dynamicObjects)
{
  std::lock_guard guard{m_mutex};
  m_laraInfo->update(lara);

  m_triggerTable->setRowCount(
    std::count_if(objects.begin(), objects.end(), [](const auto& o) { return o.second->m_isActive; })
    + std::count_if(dynamicObjects.begin(), dynamicObjects.end(), [](const auto& o) { return o->m_isActive; }));

  int row = 0;
  for(const auto& [objectId, object] : objects)
  {
    if(!object->m_isActive)
      continue;

    gsl::czstring stateStr = nullptr;
    switch(object->m_state.triggerState)
    {
    case engine::objects::TriggerState::Inactive: stateStr = "inactive"; break;
    case engine::objects::TriggerState::Active: stateStr = "active"; break;
    case engine::objects::TriggerState::Deactivated: stateStr = "deactivated"; break;
    case engine::objects::TriggerState::Invisible: stateStr = "invisible"; break;
    }
    Expects(stateStr != nullptr);

    m_triggerTable->setItem(row, 0, new QTableWidgetItem(object->getNode()->getName().c_str()));
    m_triggerTable->setItem(row, 1, new QTableWidgetItem(stateStr));
    m_triggerTable->setItem(row, 2, new QTableWidgetItem(object->m_state.timer.toString().c_str()));

    ++row;
  }
  for(const auto& object : dynamicObjects)
  {
    if(!object->m_isActive)
      continue;

    gsl::czstring stateStr = nullptr;
    switch(object->m_state.triggerState)
    {
    case engine::objects::TriggerState::Inactive: stateStr = "inactive"; break;
    case engine::objects::TriggerState::Active: stateStr = "active"; break;
    case engine::objects::TriggerState::Deactivated: stateStr = "deactivated"; break;
    case engine::objects::TriggerState::Invisible: stateStr = "invisible"; break;
    }
    Expects(stateStr != nullptr);

    m_triggerTable->setItem(row, 0, new QTableWidgetItem(object->getNode()->getName().c_str()));
    m_triggerTable->setItem(row, 1, new QTableWidgetItem(stateStr));
    m_triggerTable->setItem(row, 2, new QTableWidgetItem(object->m_state.timer.toString().c_str()));

    ++row;
  }
}
} // namespace ui::debug
