#pragma once

#include "widget.h"

#include <functional>
#include <gslu.h>

namespace ui::widgets
{
class ListBox;
class Checkbox;

class CheckListBox : public Widget
{
public:
  // getter, toggler, checkbox
  using CheckBoxData = std::tuple<std::function<bool()>, std::function<void()>, std::shared_ptr<Checkbox>>;

  explicit CheckListBox();
  ~CheckListBox() override;

  [[nodiscard]] glm::ivec2 getPosition() const override;

  [[nodiscard]] glm::ivec2 getSize() const override;
  void setPosition(const glm::ivec2& position) override;

  void setSize(const glm::ivec2& size) override;

  void update(bool hasFocus) override;

  void fitToContent() override;

  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override;

  gslu::nn_shared<Checkbox>
    addSetting(const gslu::nn_shared<Widget>& content, std::function<bool()>&& getter, std::function<void()>&& toggler);

  gslu::nn_shared<Checkbox>
    addSetting(const std::string& label, std::function<bool()>&& getter, std::function<void()>&& toggler);

  [[nodiscard]] const CheckBoxData& getSelected() const;

  [[nodiscard]] size_t getSelectedIndex() const;

  [[nodiscard]] size_t getEntryCount() const;

  void setSelectedEntry(size_t selected);

  bool nextEntry();

  bool prevEntry();

private:
  gslu::nn_shared<ListBox> m_listBox;
  std::vector<CheckBoxData> m_checkboxes;
};
} // namespace ui::widgets
