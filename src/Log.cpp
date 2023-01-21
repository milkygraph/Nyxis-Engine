#include "Log.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace ve
{
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;

	void Log::init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("Vulkan Engine");
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->flush_on(spdlog::level::trace);
	}
}