#include "Core/Application.hpp"

int main()
{
    Nyxis::Application* app = Nyxis::Application::GetInstance();
    try
    {
        app->Run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
	delete app;
    return 0;
}