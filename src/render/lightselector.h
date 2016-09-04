#pragma once

#include "level/level.h"

namespace render
{
    //! @todo Re-enable later
#if 0
    class LightSelector final : public irr::scene::ILightManager
    {
    private:
        level::Level& m_level;
        gsl::not_null<irr::scene::ISceneManager*> m_manager;

    public:
        explicit LightSelector(level::Level& level, gsl::not_null<irr::scene::ISceneManager*> mgr)
            : m_level(level)
              , m_manager(mgr)
        {
        }

        void OnPreRender(irr::core::array<irr::scene::ISceneNode*>& lightList) override
        {
            for( irr::u32 i = 0; i < lightList.size(); ++i )
                lightList[i]->setVisible(false);
            //if(node->getType() != irr::scene::ESNT_ANIMATED_MESH && node->getType() != irr::scene::ESNT_BILLBOARD && node->getType() != irr::scene::ESNT_MESH)
            //    return;

            const auto laraPos = m_level.m_lara->getSceneNode()->getPosition();
            const auto room = m_level.m_lara->getCurrentRoom();
            int maxBrightness = 0;
            const loader::Light* brightestLight = nullptr;
            for( const loader::Light& light : room->lights )
            {
                auto fadeSq = light.specularFade * light.specularFade / 4096;
                const int brightness = gsl::narrow_cast<int>((0x1fff - room->darkness) + fadeSq * light.specularIntensity
                                                             / (fadeSq + laraPos.getDistanceFromSQ(light.position.toRenderSystem()) / 4096));
                if( brightness > maxBrightness )
                {
                    maxBrightness = brightness;
                    brightestLight = &light;
                }
            }
            Expects(brightestLight != nullptr);
            brightestLight->node->setVisible(true);
            m_manager->setShadowColor(irr::video::SColor(150 * maxBrightness / 4096, 0, 0, 0));
        }

        void OnPostRender() override
        {
            // nop
        }

        void OnRenderPassPreRender(irr::scene::E_SCENE_NODE_RENDER_PASS /*renderPass*/) override
        {
        }

        void OnRenderPassPostRender(irr::scene::E_SCENE_NODE_RENDER_PASS /*renderPass*/) override
        {
        }

        void OnNodePreRender(irr::scene::ISceneNode* /*node*/) override
        {
        }

        void OnNodePostRender(irr::scene::ISceneNode* /*node*/) override
        {
            // nop
        }
    };
#endif
}
