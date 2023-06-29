#pragma once
#include "Nyxispch.hpp"

namespace Nyxis
{
	using LogLevel = spdlog::level::level_enum;

	struct LogText
	{
		spdlog::level::level_enum level;
		const std::string text;
	};

	// a wrapper class for the log messages
	class LogBuffer
	{
	public:
		LogBuffer() = default;
		~LogBuffer() = default;

		std::vector<LogText>& GetBuffer() { return m_Buffer; }
		void PushLogMessage(const LogLevel& level, const std::string& message);
		void Clear();

	private:
		std::vector<LogText> m_Buffer;
	};

	class Log
	{
		public:
		static void Init();
		static void Shutdown();

		static std::shared_ptr<spdlog::logger>& GetLogger()
		{ 
			return s_CoreLogger; 
		}

		static void PushLogMessage(LogLevel level, const std::string& message);

		static LogBuffer& GetLogBuffer()
		{
			return s_LogBuffer;
		}

		private:
		static void LoggingThreadFunction();

		static inline LogBuffer s_LogBuffer;
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