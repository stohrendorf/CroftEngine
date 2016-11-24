#pragma once

#include "Base.h"

#include <memory>

namespace gameplay
{
    class RenderContext;

    class Drawable : public std::enable_shared_from_this<Drawable>
    {
    public:
        explicit Drawable();

        virtual ~Drawable();

        virtual void draw(RenderContext& context) = 0;
    };
}
