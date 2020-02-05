#include "larainfowidget.h"

#include <QGroupBox>
#include <QVBoxLayout>

namespace ui::debug
{
LaraInfoWidget::LaraInfoWidget()
{
  setLayout(new QVBoxLayout());

  auto box = new QGroupBox("Animation");
  box->setLayout(new QVBoxLayout());
  box->layout()->addWidget(m_anim);
  layout()->addWidget(box);

  box = new QGroupBox("Location");
  box->setLayout(new QVBoxLayout());
  box->layout()->addWidget(m_location);
  box->layout()->addWidget(m_rotation);
  layout()->addWidget(box);

  box = new QGroupBox("Physics");
  box->setLayout(new QVBoxLayout());
  box->layout()->addWidget(m_speedFwd);
  box->layout()->addWidget(m_fallSpeed);
  layout()->addWidget(box);

  auto armsWidget = new QWidget();
  armsWidget->setLayout(new QHBoxLayout());
  armsWidget->layout()->addWidget(m_aimInfoLeft);
  armsWidget->layout()->addWidget(m_aimInfoRight);
  layout()->addWidget(armsWidget);

  box = new QGroupBox("Collision");
  box->setLayout(new QVBoxLayout());
  box->layout()->addWidget(m_axisColl);
  box->layout()->addWidget(m_currentFloor);
  box->layout()->addWidget(m_currentCeiling);
  box->layout()->addWidget(m_frontFloor);
  box->layout()->addWidget(m_frontCeiling);
  box->layout()->addWidget(m_frontLeftFloor);
  box->layout()->addWidget(m_frontLeftCeiling);
  box->layout()->addWidget(m_frontRightFloor);
  box->layout()->addWidget(m_frontRightCeiling);
  box->layout()->addWidget(m_needBottom);
  box->layout()->addWidget(m_needTop);
  box->layout()->addWidget(m_needCeiling);
  layout()->addWidget(box);
}

void LaraInfoWidget::update(const engine::objects::LaraObject& lara)
{
  m_anim->setText(QString("Animation: %1 -> %2 (%3)")
                    .arg(toString(lara.getCurrentAnimState()))
                    .arg(toString(lara.getGoalAnimState()))
                    .arg(lara.m_state.frame_number.toString().c_str()));
  m_location->setText(QString("%1 (%2 %3 %4)")
                        .arg(lara.m_state.position.room->node->getId().c_str())
                        .arg(lara.m_state.position.position.X.toString().c_str())
                        .arg(lara.m_state.position.position.Y.toString().c_str())
                        .arg(lara.m_state.position.position.Z.toString().c_str()));
  m_rotation->setText(QString("Y axis %1 deg").arg(std::lround(toDegrees(lara.m_state.rotation.Y))));
  m_speedFwd->setText(QString("Speed %1").arg(lara.m_state.speed.toString().c_str()));
  m_fallSpeed->setText(QString("Fall speed %1").arg(lara.m_state.fallspeed.toString().c_str()));

  m_aimInfoLeft->update(lara.leftArm);
  m_aimInfoRight->update(lara.rightArm);

  m_axisColl->setText(QString("AxisColl: %1").arg(toString(lara.lastUsedCollisionInfo.collisionType)));
  m_currentFloor->setText(
    QString("Current floor: %1").arg(lara.lastUsedCollisionInfo.mid.floorSpace.y.toString().c_str()));
  m_currentCeiling->setText(
    QString("Current ceiling: %1").arg(lara.lastUsedCollisionInfo.mid.ceilingSpace.y.toString().c_str()));
  m_frontFloor->setText(
    QString("Front floor: %1").arg(lara.lastUsedCollisionInfo.front.floorSpace.y.toString().c_str()));
  m_frontCeiling->setText(
    QString("Front ceiling: %1").arg(lara.lastUsedCollisionInfo.front.ceilingSpace.y.toString().c_str()));
  m_frontLeftFloor->setText(
    QString("Front left floor: %1").arg(lara.lastUsedCollisionInfo.frontLeft.floorSpace.y.toString().c_str()));
  m_frontLeftCeiling->setText(
    QString("Front left ceiling: %1").arg(lara.lastUsedCollisionInfo.frontLeft.ceilingSpace.y.toString().c_str()));
  m_frontRightFloor->setText(
    QString("Front right floor: %1").arg(lara.lastUsedCollisionInfo.frontRight.floorSpace.y.toString().c_str()));
  m_frontRightCeiling->setText(
    QString("Front right ceiling: %1").arg(lara.lastUsedCollisionInfo.frontRight.ceilingSpace.y.toString().c_str()));
  m_needBottom->setText(
    QString("Need bottom: %1").arg(lara.lastUsedCollisionInfo.badPositiveDistance.toString().c_str()));
  m_needTop->setText(QString("Need top: %1").arg(lara.lastUsedCollisionInfo.badNegativeDistance.toString().c_str()));
  m_needCeiling->setText(
    QString("Need ceiling: %1").arg(lara.lastUsedCollisionInfo.badCeilingDistance.toString().c_str()));
}
} // namespace ui::debug
