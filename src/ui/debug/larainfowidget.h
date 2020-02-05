#pragma once

#include "engine/objects/laraobject.h"
#include "laraaiminfo.h"

#include <QLabel>
#include <QWidget>

namespace ui::debug
{
class LaraInfoWidget : public QWidget
{
  Q_OBJECT
public:
  explicit LaraInfoWidget();

  void update(const engine::objects::LaraObject& lara);

private:
  QLabel* m_anim = new QLabel();
  QLabel* m_location = new QLabel();
  QLabel* m_rotation = new QLabel();
  QLabel* m_speedFwd = new QLabel();
  QLabel* m_fallSpeed = new QLabel();
  LaraAimInfo* m_aimInfoLeft = new LaraAimInfo("Left Arm");
  LaraAimInfo* m_aimInfoRight = new LaraAimInfo("Right Arm");
  QLabel* m_axisColl = new QLabel();
  QLabel* m_currentFloor = new QLabel();
  QLabel* m_currentCeiling = new QLabel();
  QLabel* m_frontFloor = new QLabel();
  QLabel* m_frontCeiling = new QLabel();
  QLabel* m_frontLeftFloor = new QLabel();
  QLabel* m_frontLeftCeiling = new QLabel();
  QLabel* m_frontRightFloor = new QLabel();
  QLabel* m_frontRightCeiling = new QLabel();
  QLabel* m_needBottom = new QLabel();
  QLabel* m_needTop = new QLabel();
  QLabel* m_needCeiling = new QLabel();
};
} // namespace ui::debug
