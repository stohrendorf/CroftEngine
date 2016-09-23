#include "Base.h"
#include "MathUtil.h"


namespace gameplay
{
    void MathUtil::smooth(float* x, float target, float elapsedTime, float responseTime)
    {
        BOOST_ASSERT(x);

        if( elapsedTime > 0 )
        {
            *x += (target - *x) * elapsedTime / (elapsedTime + responseTime);
        }
    }


    void MathUtil::smooth(float* x, float target, float elapsedTime, float riseTime, float fallTime)
    {
        BOOST_ASSERT(x);

        if( elapsedTime > 0 )
        {
            float delta = target - *x;
            *x += delta * elapsedTime / (elapsedTime + (delta > 0 ? riseTime : fallTime));
        }
    }
}
