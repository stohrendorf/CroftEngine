#pragma once

#include "widget.h"

#include <glm/glm.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <string>

namespace ui
{
class Ui;
} // namespace ui

namespace engine
{
class Presenter;
}

namespace ui::widgets
{
class Label;

class MessageBox : public Widget
{
public:
  explicit MessageBox(const std::string& label);
  ~MessageBox() override;
  void update(bool hasFocus) override;
  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;

  void setConfirmed(bool confirmed)
  {
    m_confirmed = confirmed;
  }

  [[nodiscard]] bool isConfirmed() const
  {
    return m_confirmed;
  }

  void setPosition(const glm::ivec2& position) override;

  [[nodiscard]] glm::ivec2 getPosition() const override;
  [[nodiscard]] glm::ivec2 getSize() const override;
  void setSize(const glm::ivec2& size) override;
  void fitToContent() override;

private:
  glm::ivec2 m_position{0, 0};
  bool m_confirmed = false;
  std::shared_ptr<Label> m_question;
  std::shared_ptr<Label> m_yes;
  std::shared_ptr<Label> m_no;
};
} // namespace ui::widgets
