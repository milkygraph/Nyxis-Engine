#include "Core/Log.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Nyxis
{
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;

	void Log::init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("Nyxis Engine");
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->flush_on(spdlog::level::trace);
	}
}