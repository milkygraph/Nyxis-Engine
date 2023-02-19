#pragma once
#include "vepch.hpp"

namespace Nyxis
{
	class Log
	{
		public:
		static void init();

		inline static std::shared_ptr<spdlog::logger>& GetLogger()
		{ 
			return s_CoreLogger; 
		}

		private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;

	};
}

#define LOG_TRACE(...) ::Nyxis::Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...) ::Nyxis::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) ::Nyxis::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) ::Nyxis::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::Nyxis::Log::GetLogger()->critical(__VA_ARGS__)