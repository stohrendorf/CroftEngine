#include "wolf.h"

#include "engine/heightinfo.h"
#include "level/level.h"
#include "engine/laranode.h"
#include "engine/pathfinding.h"

namespace engine
{
    namespace items
    {
        void Wolf::updateImpl(const std::chrono::microseconds& deltaTime, const boost::optional<FrameChangeType>& /*frameChangeType*/)
        {
            engine::LotInfo path;
            core::ExactTRCoordinates target;
            path.calculateTarget(target, *this, *getLevel().m_lara);

            if(getPosition() == target)
                return;

            static constexpr const float speed = 20;

            BOOST_LOG_TRIVIAL(debug) << "Path info for " << getId();
            BOOST_LOG_TRIVIAL(debug) << "  - Position: " << getPosition().X << "/" << getPosition().Y << "/" << getPosition().Z;
            BOOST_LOG_TRIVIAL(debug) << "  - Lara: " << getLevel().m_lara->getPosition().X << "/" << getLevel().m_lara->getPosition().Y << "/" << getLevel().m_lara->getPosition().Z;
            BOOST_LOG_TRIVIAL(debug) << "  - Target: " << target.X << "/" << target.Y << "/" << target.Z;
            BOOST_LOG_TRIVIAL(debug) << "  - Distance to target: " << getPosition().distanceTo(target);
            const auto f = glm::clamp(core::makeInterpolatedValue(speed).getScaled(deltaTime) / getPosition().distanceTo(target), 0.0f, 0.99f);
            BOOST_LOG_TRIVIAL(debug) << "  - f: " << f;
            const auto d = target - getPosition();
            BOOST_LOG_TRIVIAL(debug) << "  - d: " << d.X << "/" << d.Y << "/" << d.Z;
            const auto tmp = core::ExactTRCoordinates(d.toRenderSystem() * f);
            BOOST_LOG_TRIVIAL(debug) << "  - d * f: " << tmp.X << "/" << tmp.Y << "/" << tmp.Z;

            move(d.toRenderSystem() * f);
            updateRoomBinding();
        }


        void Wolf::onFrameChanged(FrameChangeType /*frameChangeType*/)
        {
        }
    }
}