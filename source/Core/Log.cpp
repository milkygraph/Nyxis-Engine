#include "Core/Log.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Nyxis
{
	void Log::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("Nyxis Engine");
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->flush_on(spdlog::level::trace);

		s_LoggingThread = std::thread(Log::LoggingThreadFunction);
	}

	void Log::Shutdown()
	{
		s_LoggingThreadRunning = false;
		s_ConditionVariable.notify_all();
		s_LoggingThread.join();
	}

	void Log::LoggingThreadFunction()
	{
		while (s_LoggingThreadRunning)
		{
			std::unique_lock<std::mutex> lock(s_Mutex);
			s_ConditionVariable.wait(lock);
			while (!s_LogQueue.empty())
			{
				auto [level, message] = s_LogQueue.front();
				s_LogQueue.pop();
				switch (level)
				{
				case spdlog::level::trace:
					s_CoreLogger->trace(message);
					break;
				case spdlog::level::info:
					s_CoreLogger->info(message);
					break;
				case spdlog::level::warn:
					s_CoreLogger->warn(message);
					break;
				case spdlog::level::err:
					s_CoreLogger->error(message);
					break;
				case spdlog::level::critical:
					s_CoreLogger->critical(message);
					break;
				}
			}
		}
	}
}