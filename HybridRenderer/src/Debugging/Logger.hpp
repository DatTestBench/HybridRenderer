#ifndef LOGGER_HPP
#define LOGGER_HPP
#include "Helpers/Singleton.hpp"

#include <array>
#include <list>
#include <sstream>

#define LOG_CONSOLE_ONLY 0
#define LOG_LOGGER_ONLY 1
#define LOG_BOTH 0


#if LOG_CONSOLE_ONLY
	#define LOG(level, header, input) std::cout << Logger::GetInstance()->RawOutput(level, header) << input << "\n";
#elif LOG_LOGGER_ONLY
	#define LOG(level, header, input) Logger::GetInstance()->Log<level>(header) << input;
#elif LOG_BOTH
	#define LOG(level, header, input) std::cout << Logger::GetInstance()->RawOutput(level, header) << input << "\n"; \
									Logger::GetInstance()->Log<level>(header) << input;
#endif



enum LogLevel : uint16_t
{
	LEVEL_SUCCESS,
	LEVEL_DEBUG,
	LEVEL_INFO,
	LEVEL_WARNING,
	LEVEL_ERROR,
	LEVEL_FULL // Only used to for displaying in the log window, should not be passed to logentries
};

enum LogArgument : uint16_t
{
	LOG_IMGUI,
	LOG_CONSOLE
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

	{
	}
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
		static_assert(Level != LEVEL_FULL, "LEVEL_FULL is not a valid LogLevel");

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

	std::string  RawOutput(const LogLevel level, const std::string& header) const 
	{
		return std::string("[" + m_LevelTags.at(level) + "] " + header + " > ");
	}
	void OutputLog();

private:


	std::list<LogEntry> m_LogList;
	bool m_ShowHeaders = true;
	LogLevel m_CurrentLevel = LEVEL_FULL;

	const std::array<ImVec4, 6> m_ImGuiColors
	{
		ImVec4{ 0.f, 1.f, 0.f, 1.f }, // SUCCESS
		ImVec4{ 1.f, 0.f, 1.f, 1.f }, // DEBUG
		ImVec4{ 1.f, 1.f, 1.f, 1.f }, // INFO
		ImVec4{ 1.f, 1.f, 0.f, 1.f }, // WARNING
		ImVec4{ 1.f, 0.f, 0.f, 1.f },	// ERROR
		ImVec4{ 1.f, 1.f, 1.f, 1.f }	// DEFAULT
	};

	const std::array<std::string, 6> m_LevelTags
	{
		"SUCCESS",
		"DEBUG",
		"INFO",
		"WARNING",
		"ERROR",
		"FULL" // Only used to for displaying in the log window, should not be passed to logentries
	};
};

#endif // !LOGGER_HPP