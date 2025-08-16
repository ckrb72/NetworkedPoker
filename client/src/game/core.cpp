#include "core.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>



static void framebuffer_resize_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

namespace game
{
    namespace core
    {
        window::window(uint32_t width, uint32_t height)
        :width(width), height(height)
        {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            this->handle = glfwCreateWindow(width, height, "App", nullptr, nullptr);
            
            glfwSetFramebufferSizeCallback(this->handle, framebuffer_resize_callback);

            glfwMakeContextCurrent(this->handle);
        }

        void window::resize(uint32_t width, uint32_t height)
        {
            glfwSetWindowSize(this->handle, width, height);
            
            int w, h;
            glfwGetFramebufferSize(this->handle, &w, &h);

            glViewport(0, 0, w, h);
        }
        
        static void mouse_callback(GLFWwindow *window, int button, int action, int mods)
        {
            game::core::input* input = (game::core::input*)glfwGetWindowUserPointer(window);
        
            if(action == GLFW_PRESS) input->mouse.buttons[button] = game::core::key_state::PRESSED;
            else if(action == GLFW_RELEASE) input->mouse.buttons[button] = game::core::key_state::RELEASED;
        }


        input::input(const GLFWwindow& window)
        :window(window)
        {
            glfwSetWindowUserPointer((GLFWwindow*)&window, this);
            glfwSetMouseButtonCallback((GLFWwindow*)&window, mouse_callback);
        }

        void input::update()
        {
            // Update key states and stuff here
        }
        
        void input::poll()
        {
            glfwPollEvents();
        }

    }
}