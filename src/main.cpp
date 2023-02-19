#include "app.hpp"

#include <iostream>
#include <stdexcept>
#include <cstdlib>

int main()
{
    Nyxis::App* app = Nyxis::App::getInstance();
    try
    {
        app->run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
	delete app;
    return 0;
}