#pragma once

#include "widget.h"

#include <cstddef>
#include <glm/vec2.hpp>
#include <gslu.h>
#include <string>
#include <vector>

namespace engine
{
class Presenter;
}

namespace ui
{
class Ui;
class Text;
} // namespace ui

namespace ui::widgets
{
class Tab : public ui::widgets::Widget
{
public:
  explicit Tab(const std::string& title);
  ~Tab() override;

  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;
  [[nodiscard]] glm::ivec2 getSize() const override
  {
    return m_size;
  }
  [[nodiscard]] glm::ivec2 getPosition() const override
  {
    return m_position;
  }
  void setPosition(const glm::ivec2& position) override
  {
    m_position = position;
  }
  void setSize(const glm::ivec2& size) override
  {
    m_size = size;
  }
  void update(bool hasFocus) override;
  void fitToContent() override;

private:
  bool m_hasFocus = false;
  gslu::nn_unique<Text> m_title;
  glm::ivec2 m_size{0, 0};
  glm::ivec2 m_position{0, 0};
};

class TabBox : public Widget
{
public:
  explicit TabBox();
  ~TabBox() override;

  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;
  [[nodiscard]] glm::ivec2 getSize() const override
  {
    return m_size;
  }
  [[nodiscard]] glm::ivec2 getPosition() const override
  {
    return m_position;
  }
  void setPosition(const glm::ivec2& position) override
  {
    m_position = position;
  }
  void setSize(const glm::ivec2& size) override
  {
    m_size = size;
  }
  void update(bool hasFocus) override;
  void fitToContent() override;

  void addTab(const gslu::nn_shared<Tab>& tab, const gslu::nn_shared<Widget>& content);
  void nextTab()
  {
    if(m_selectedTabIndex < m_tabs.size() - 1)
    {
      m_selectedTabIndex++;
    }
  }
  void prevTab()
  {
    if(m_selectedTabIndex > 0)
    {
      m_selectedTabIndex--;
    }
  }
  [[nodiscard]] size_t getSelectedTab() const
  {
    return m_selectedTabIndex;
  }

private:
  struct ContainerContent
  {
    gslu::nn_shared<Widget> container;
    gslu::nn_shared<Widget> content;
  };

  size_t m_selectedTabIndex{0};
  std::vector<ContainerContent> m_tabs;
  glm::ivec2 m_position{0, 0};
  glm::ivec2 m_size{0, 0};
};

} // namespace ui::widgets