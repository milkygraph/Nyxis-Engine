#include "app.hpp"
#include <iostream>
namespace ve
{
    void App::run()
    {
        while (!pWindow.shouldClose())
        {
            glfwPollEvents();
        }
        std::cout << "Bye" << std::endl;

    }
} // namespace ve