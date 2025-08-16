#pragma once
#include <glm/glm.hpp>

namespace game
{
    namespace component
    {
        class transform
        {
            public:
                glm::vec2 pos;
                glm::vec2 scale;
        };
    }
}