#pragma once
#include <cstdint>

struct GLFWwindow;

namespace game
{
    namespace core
    {

        class window
        {
            private:
                uint32_t width, height;
                GLFWwindow* handle;
            public:
                window(uint32_t width, uint32_t height);
                void resize(uint32_t width, uint32_t height);
        };

        enum mouse_button
        {
            LEFT,
            RIGHT,
            MIDDLE
        };

        enum key_state
        {
            RELEASED,
            PRESSED,
            HELD
        };

        class input
        {
            private:
                const GLFWwindow& window;

                // key_state keys[]

                struct mouse
                {
                    key_state buttons[3];
                    double mx, my;
                    double dmx, dmy;
                };

                input::mouse mouse;

            public:
                input(const GLFWwindow& window);
                void update();
                void poll();
                key_state key_state(int key);

            friend void mouse_callback(GLFWwindow *window, int button, int action, int mods);
        };

        class renderer
        {

        };
    }
}