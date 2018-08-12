#include "particle.h"

#include "level/level.h"

namespace engine
{

void FX::initDrawables(const level::Level& level, const gsl::not_null<std::shared_ptr<gameplay::Material>>& material)
{
    auto it = level.m_animatedModels.find( object_number );
    BOOST_ASSERT( it != level.m_animatedModels.end() );
    const std::unique_ptr<loader::SkeletalModelType>& modelType = it->second;

    if( modelType->nmeshes == 0 )
    {
        return;
    }

    if( modelType->nmeshes > 0 )
    {
        BOOST_ASSERT( modelType->frame_number + modelType->nmeshes <= level.m_sprites.size() );
        for( int i = 0; i < modelType->nmeshes; ++i )
        {
            auto model = level.m_models2[modelType->frame_number + i].get();
            m_drawables.push_back( model );
        }
    }
    else if( modelType->nmeshes < 0 )
    {
        BOOST_ASSERT( modelType->frame_number - modelType->nmeshes <= level.m_sprites.size() );

        shade = 4096;

        for( int i = 0; i > modelType->nmeshes; --i )
        {
            const loader::Sprite& spr = level.m_sprites[modelType->frame_number - i];

            auto sprite = std::make_shared<gameplay::Sprite>(
                    spr.left_side,
                    spr.bottom_side,
                    spr.right_side - spr.left_side,
                    spr.top_side - spr.bottom_side,
                    spr.t0,
                    spr.t1,
                    material,
                    gameplay::Sprite::Axis::Y
            );
            m_drawables.emplace_back( sprite );
        }
    }
}
}