#include "Core/Application.hpp"
#include "Core/Log.hpp"

int main()
{
	Nyxis::Log::Init();
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
	Nyxis::Log::Shutdown();
    return 0;
}