#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace ui
{
struct Label;
class Ui;
} // namespace ui

namespace engine
{
class Presenter;
}

namespace menu::widgets
{
class Checkbox
{
public:
  explicit Checkbox(const glm::ivec2& position, const std::string& label, int width = 0);
  ~Checkbox();
  void update(bool selected);
  void draw(ui::Ui& ui, const engine::Presenter& presenter);

  void setChecked(bool checked)
  {
    m_checked = checked;
  }

  void setPosition(const glm::ivec2& position);

private:
  std::unique_ptr<ui::Label> m_label;
  bool m_checked = false;
};
} // namespace menu::widgets
