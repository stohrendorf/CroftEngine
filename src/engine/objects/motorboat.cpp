#include "motorboat.h"

namespace engine::objects
{
void Motorboat::update()
{
  if(m_state.current_anim_state == 1_as)
  {
    m_state.goal_anim_state = 2_as;
  }
  else if(m_state.current_anim_state == 2_as)
  {
    m_state.goal_anim_state = 3_as;
  }
  else if(m_state.current_anim_state == 3_as)
  {
    kill();
  }

  ModelObject::update();
}
} // namespace engine::objects
