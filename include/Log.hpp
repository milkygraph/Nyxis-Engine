#include <spdlog/spdlog.h>

namespace ve
{
	class Log
	{
		public:
		static void init();

		inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_CoreLogger; }

		private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;

	};
}

#define LOG_TRACE(...) ::ve::Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...) ::ve::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) ::ve::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) ::ve::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::ve::Log::GetLogger()->critical(__VA_ARGS__)