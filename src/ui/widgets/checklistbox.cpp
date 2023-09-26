#include "checklistbox.h"

#include "checkbox.h"
#include "label.h"
#include "listbox.h"

namespace ui::widgets
{

CheckListBox::CheckListBox()
    : m_listBox{std::make_shared<ListBox>()}
{
}

CheckListBox::~CheckListBox() = default;

glm::ivec2 CheckListBox::getPosition() const
{
  return m_listBox->getPosition();
}

glm::ivec2 CheckListBox::getSize() const
{
  return m_listBox->getSize();
}

void CheckListBox::setPosition(const glm::ivec2& position)
{
  m_listBox->setPosition(position);
}

void CheckListBox::setSize(const glm::ivec2& size)
{
  m_listBox->setSize(size);
}

void CheckListBox::update(bool hasFocus)
{
  m_listBox->update(hasFocus);
}

void CheckListBox::fitToContent()
{
  for(const auto& [getter, toggler, checkbox] : m_checkboxes)
    checkbox->fitToContent();

  m_listBox->fitToContent();
}

void CheckListBox::draw(Ui& ui, const engine::Presenter& presenter) const
{
  m_listBox->draw(ui, presenter);
}

gslu::nn_shared<Checkbox> CheckListBox::addSetting(const gslu::nn_shared<Widget>& content,
                                                   std::function<bool()>&& getter,
                                                   std::function<void()>&& toggler)
{
  auto checkbox = gsl::make_shared<Checkbox>(content);
  checkbox->setChecked(getter());
  checkbox->fitToContent();
  m_listBox->append(checkbox);
  m_checkboxes.emplace_back(std::move(getter), std::move(toggler), checkbox);
  return checkbox;
}

gslu::nn_shared<Checkbox>
  CheckListBox::addSetting(const std::string& label, std::function<bool()>&& getter, std::function<void()>&& toggler)
{
  return addSetting(gsl::make_shared<Label>(label), std::move(getter), std::move(toggler));
}

const CheckListBox::CheckBoxData& CheckListBox::getSelected() const
{
  return m_checkboxes.at(m_listBox->getSelected());
}

size_t CheckListBox::getSelectedIndex() const
{
  return m_listBox->getSelected();
}

size_t CheckListBox::getEntryCount() const
{
  return m_listBox->getWidgets().size();
}

void CheckListBox::setSelectedEntry(size_t selected)
{
  m_listBox->setSelected(selected);
}

bool CheckListBox::nextEntry()
{
  return m_listBox->nextEntry();
}

bool CheckListBox::prevEntry()
{
  return m_listBox->prevEntry();
}
} // namespace ui::widgets
