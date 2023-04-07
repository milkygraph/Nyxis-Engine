#pragma once
#include "Nyxispch.hpp"

namespace Nyxis
{
	class Log
	{
		public:
		static void Init();
		static void Shutdown();

		static std::shared_ptr<spdlog::logger>& GetLogger()
		{ 
			return s_CoreLogger; 
		}

		static void PushLogMessage(spdlog::level::level_enum level, const std::string message)
		{
			std::lock_guard<std::mutex> lock(s_Mutex);
			s_LogQueue.push({ level, message.data() });
			s_ConditionVariable.notify_one();
		}

		private:
		static void LoggingThreadFunction();

		static inline std::shared_ptr<spdlog::logger> s_CoreLogger;
		static inline std::queue < std::tuple < spdlog::level::level_enum, std::string >> s_LogQueue;
		static inline std::mutex s_Mutex;
		static inline std::condition_variable s_ConditionVariable;
		static inline std::thread s_LoggingThread;
		static inline bool s_LoggingThreadRunning = true;
	};

}

#define LOG_TRACE(...) ::Nyxis::Log::PushLogMessage(spdlog::level::trace, fmt::format(__VA_ARGS__))
#define LOG_INFO(...) ::Nyxis::Log::PushLogMessage(spdlog::level::info, fmt::format(__VA_ARGS__))
#define LOG_ERROR(...) ::Nyxis::Log::PushLogMessage(spdlog::level::err, fmt::format(__VA_ARGS__))
#define LOG_WARN(...) ::Nyxis::Log::PushLogMessage(spdlog::level::warn, fmt::format(__VA_ARGS__))
#define LOG_CRITICAL(...) ::Nyxis::Log::PushLogMessage(spdlog::level::critical, fmt::format(__VA_ARGS__))