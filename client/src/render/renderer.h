#pragma once
#include <glm/glm.hpp>

namespace render
{
    struct character_vertex
    {
        glm::vec2 pos;
        glm::vec2 tex;
    };

    struct character_info
    {
        int atlas_offsetx;
        int atlas_offsety;
        unsigned int width, height;
        int bearingx, bearingy;
        int advance;
    };
};