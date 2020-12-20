#ifndef LOGGER_HPP
#define LOGGER_HPP

// General Includes
#include <list>
#include <array>
#include <sstream>

// Project Includes
#include "Helpers/Singleton.hpp"
#include "Helpers/magic_enum.hpp"

// todo(matthieu) clean all of this up

#define DEBUG_OVERRIDE 1

#define LOG_CONSOLE_ONLY 1
#define LOG_LOGGER_ONLY 2
#define LOG_BOTH 3

#define LOG_OUTPUT LOG_LOGGER_ONLY

#if _DEBUG || DEBUG_OVERRIDE
	#if LOG_OUTPUT == LOG_CONSOLE_ONLY
		#define LOG(level, header, input) std::cout << Logger::GetInstance()->RawOutput(LogLevel::level, header) << input << "\n";
	#elif LOG_OUTPUT == LOG_LOGGER_ONLY
		#define LOG(level, header, input) Logger::GetInstance()->Log<LogLevel::level>(header) << input;
	#elif LOG_OUTPUT == LOG_BOTH
		#define LOG(level, header, input) std::cout << Logger::GetInstance()->RawOutput(LogLevel::level, header) << input << "\n"; \
									Logger::GetInstance()->Log<LogLevel::level>(header) << input;
	#endif
#else
	#define LOG(level, header, input) {}
#endif

enum class LogLevel : int16_t
{
	Success = 0,
	LEVEL_SUCCESS = Success,
	Debug = 1,
	LEVEL_DEBUG = Debug,
	Info = 2,
	LEVEL_INFO = Info,
	Warning = 3,
	LEVEL_WARNING = Warning,
	Error = 4,
	LEVEL_ERROR = Error,
	Full = 5, // Only used to for displaying in the log window, should not be passed to logentries
	LEVEL_FULL = Full // Only used to for displaying in the log window, should not be passed to logentries
};

struct LogEntry
{
	std::string header;
	std::stringstream message;
	LogLevel level;
	bool markedForClear;
	LogEntry(const std::string& log, const LogLevel lvl)
		: header(log)
		, level(lvl)
		, markedForClear(false)
	{}
};

class Logger final : public Singleton<Logger>
{
public:

	explicit Logger(Token) {}
	
	/**
	 * Log Function 
	 * @param Level (Template) LogLevel
	 * @param header Name of the scope this log was called in
	 * */
	template<LogLevel Level>
	static Logger& Log(const std::string& header = "")
	{
		static_assert(Level != LogLevel::LEVEL_FULL, "LEVEL_FULL is not a valid LogLevel");

		GetInstance()->m_LogList.emplace_back(LogEntry(header, Level));
		return *GetInstance();
	}

	/**
	 * Ostream for extra log messages
	 * @param log Log message
	 * */
	template<class T>
	Logger& operator<<(const T& log)
	{
		m_LogList.back().message << log;
		return *GetInstance();
	}

	/**
	 * Returns string with loglevel + header structure
	 * @returns string with loglevel + header
	 * */
	std::string RawOutput(const LogLevel level, const std::string& header) const 
	{
		return std::string("[" + std::string(magic_enum::enum_name(level)) + "] " + header + " > ");
	}

	/**
	 * ImGui code to output the logger window
	 * */
	void OutputLog() noexcept;

private:
	
	std::list<LogEntry> m_LogList;
	bool m_ShowHeaders = true;
	LogLevel m_CurrentLevel = LogLevel::LEVEL_FULL;

	const std::array<ImVec4, 6> m_ImGuiColors
	{
		ImVec4{ 0.f, 1.f, 0.f, 1.f }, // SUCCESS
		ImVec4{ 1.f, 0.f, 1.f, 1.f }, // DEBUG
		ImVec4{ 1.f, 1.f, 1.f, 1.f }, // INFO
		ImVec4{ 1.f, 1.f, 0.f, 1.f }, // WARNING
		ImVec4{ 1.f, 0.f, 0.f, 1.f },	// ERROR
		ImVec4{ 1.f, 1.f, 1.f, 1.f }	// DEFAULT
	};
};

#endif // !LOGGER_HPP