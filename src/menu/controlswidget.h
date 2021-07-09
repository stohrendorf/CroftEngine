#pragma once

#include "ui/widgets/widget.h"

#include <functional>
#include <memory>
#include <string>

namespace hid
{
enum class Action;
class InputHandler;
} // namespace hid

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
  explicit ControlsWidget(
    const std::string& title,
    const hid::InputHandler& inputHandler,
    std::function<std::shared_ptr<ui::widgets::Widget>(const hid::InputHandler&, hid::Action)> factory);

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

  void updateBindings(const hid::InputHandler& inputHandler);

private:
  std::shared_ptr<ui::widgets::GridBox> m_content;
  std::shared_ptr<ui::widgets::GroupBox> m_container;
  std::vector<std::shared_ptr<ui::widgets::GridBox>> m_controlGroups;
  std::function<std::shared_ptr<Widget>(const hid::InputHandler&, hid::Action)> m_factory;
};
} // namespace menu
