#pragma once

#include "ui/widgets/widget.h"

#include <functional>

namespace hid
{
enum class Action;
}

namespace ui::widgets
{
class GridBox;
class GroupBox;
} // namespace ui::widgets

namespace menu
{
class ControlsWidget final : public ui::widgets::Widget
{
public:
  static constexpr size_t Columns = 6;

  explicit ControlsWidget(const std::string& title,
                          const std::function<std::shared_ptr<ui::widgets::Widget>(hid::Action)>& factory);

  [[nodiscard]] glm::ivec2 getPosition() const override;

  [[nodiscard]] glm::ivec2 getSize() const override;

  void setPosition(const glm::ivec2& position) override;

  void setSize(const glm::ivec2& size) override;

  void update(bool hasFocus) override;

  void fitToContent() override;

  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;

  [[nodiscard]] const std::shared_ptr<ui::widgets::GridBox>& getCurrentGridBox() const;

  void nextRow();

  void prevRow();

  void nextColumn();

  void prevColumn();

private:
  std::shared_ptr<ui::widgets::GridBox> m_content;
  std::shared_ptr<ui::widgets::GroupBox> m_container;
  std::vector<std::shared_ptr<ui::widgets::GridBox>> m_controlGroups;
};
} // namespace menu
