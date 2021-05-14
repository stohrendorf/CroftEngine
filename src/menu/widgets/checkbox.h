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
  explicit Checkbox(const glm::ivec2& pos, const std::string& label, const glm::ivec2& size = glm::ivec2{0, 0});
  ~Checkbox();
  void draw(ui::Ui& ui, const engine::Presenter& presenter, bool selected);

  void setChecked(bool checked)
  {
    m_checked = checked;
  }

private:
  std::unique_ptr<ui::Label> m_label;
  bool m_checked = false;
};
} // namespace menu::widgets
