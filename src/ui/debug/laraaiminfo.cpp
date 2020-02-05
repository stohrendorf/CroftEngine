#include "laraaiminfo.h"

#include <QVBoxLayout>

namespace ui::debug
{
LaraAimInfo::LaraAimInfo(const QString& title)
    : QGroupBox{title}
{
  setLayout(new QVBoxLayout());

  layout()->addWidget(m_aiming);
  layout()->addWidget(m_rotation);
}

void LaraAimInfo::update(const engine::objects::LaraObject::AimInfo& aimInfo)
{
  m_aiming->setText(aimInfo.aiming ? "Aiming" : "Not aiming");
  m_rotation->setText(QString("%1 %2").arg(toDegrees(aimInfo.aimRotation.X)).arg(toDegrees(aimInfo.aimRotation.Y)));
}
} // namespace ui::debug
