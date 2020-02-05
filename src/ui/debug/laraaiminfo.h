#pragma once

#include "engine/objects/laraobject.h"

#include <QGroupBox>
#include <QLabel>

namespace ui::debug
{
class LaraAimInfo : public QGroupBox
{
  Q_OBJECT
public:
  explicit LaraAimInfo(const QString& title);

  void update(const engine::objects::LaraObject::AimInfo& aimInfo);

private:
  QLabel* m_aiming = new QLabel();
  QLabel* m_rotation = new QLabel();
};
} // namespace ui::debug
