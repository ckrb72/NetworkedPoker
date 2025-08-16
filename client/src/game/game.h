#pragma once
#include "components.h"
#include "core.h"

namespace game
{


    class app
    {
        private:
            core::window window;
            core::input input;
            core::renderer renderer;

        public:
            app();
    };

    // all game objects derive this class, adding components as needed
    class object
    {
        public:
            component::transform transform;
            virtual void update() {}
    };

}