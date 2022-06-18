#pragma once

#include "label.h"
#include "widget.h"

#include <algorithm>
#include <functional>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <utility>
#include <vector>
namespace engine
{
class Presenter;
}

namespace ui
{
class Ui;
} // namespace ui

namespace ui::widgets
{
template<typename T>
class ValueSelector : public Widget
{
public:
  using DisplayTextProvider = std::function<std::string(const T&)>;

  explicit ValueSelector(DisplayTextProvider&& displayTextProvider,
                         std::vector<T> values,
                         Label::Alignment alignment = Label::Alignment::Left)
      : m_label{{}, alignment}
      , m_values{std::move(values)}
      , m_displayTextProvider{std::move(displayTextProvider)}
  {
    Expects(m_displayTextProvider);
    if(!m_values.empty())
      m_label.setText(m_displayTextProvider(m_values[0]));
  }
  ~ValueSelector() override = default;

  [[nodiscard]] glm::ivec2 getPosition() const override
  {
    return m_label.getPosition();
  }

  [[nodiscard]] glm::ivec2 getSize() const override
  {
    return m_label.getSize();
  }

  void setPosition(const glm::ivec2& position) override
  {
    m_label.setPosition(position);
  }

  void setSize(const glm::ivec2& size) override
  {
    m_label.setSize(size);
  }

  void update(bool hasFocus) override
  {
    m_label.update(hasFocus);
  }

  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override
  {
    m_label.draw(ui, presenter);
  }

  void fitToContent() override
  {
    m_label.fitToContent();
  }

  void selectPrev()
  {
    if(m_selectedValue == 0)
      return;

    --m_selectedValue;
    m_label.setText(m_displayTextProvider(m_values[m_selectedValue]));
  }

  void selectNext()
  {
    if(m_values.empty() || m_selectedValue == m_values.size() - 1)
      return;

    ++m_selectedValue;
    m_label.setText(m_displayTextProvider(m_values[m_selectedValue]));
  }

  [[nodiscard]] auto getSelectedValue() const
  {
    return m_values.at(m_selectedValue);
  }

  void selectValue(const T& value)
  {
    if(const auto it = std::find(m_values.begin(), m_values.end(), value); it != m_values.end())
      m_selectedValue = std::distance(m_values.begin(), it);
    m_label.setText(m_displayTextProvider(m_values[m_selectedValue]));
  }

private:
  Label m_label;
  std::vector<T> m_values;
  size_t m_selectedValue = 0;
  DisplayTextProvider m_displayTextProvider;
};
} // namespace ui::widgets
