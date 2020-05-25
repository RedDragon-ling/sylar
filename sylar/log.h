#ifndef _SYLAY_LOG_H_
#define _SYLAY_LOG_H_

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <strstream> //stringstream
#include <fstream>
#include <iostream>

namespace sylar
{

	class LogEvent
	{
	public:
		typedef std::shared_ptr<LogEvent> ptr;
		LogEvent();

	private:
		const char *m_file = nullptr; // 文件名
		int32_t m_line = 0;			  // 行号
		uint32_t m_elapse = 0;		  // 程序启动到现在的毫秒数
		uint32_t m_threadId = 0;	  // 线程ID
		uint32_t n_fiberId = 0;		  // 协程ID
		uint32_t m_time;			  // 时间戳
		std::string m_cotent;		  // 消息
	};
	// 日志级别
	class LogLevel
	{
	public:
		enum Level
		{
			DEBUG = 1,
			INFO = 2,
			WARN = 3,
			ERROR = 4,
			FATAL = 5, // 致命
		};
	};

	// 日志格式器
	class LogFormatter
	{
	public:
		typedef std::shared_ptr<LogFormatter> ptr;

		std::string format(LogEvent::ptr event);

	private:
	};

	// 日志输出地(文件输出)
	class LogAppender
	{
	public:
		typedef std::shared_ptr<LogAppender> ptr;
		virtual ~LogAppender() {}

		virtual void log(LogLevel::Level, LogEvent::ptr event) = 0;

		void setFormatter(LogFormatter::ptr val);
		LogFormatter::ptr getFormatter() const { return m_formatter; }

	protected:
		LogLevel::Level m_level;
		LogFormatter::ptr m_formatter;
	};

	// 日志器
	class Logger
	{
	public:
		typedef std::shared_ptr<Logger> ptr;

		Logger(const std::string &name = "root");
		void log(LogLevel::Level level, LogEvent::ptr event);

		void debug(LogEvent::ptr event);
		void info(LogEvent::ptr event);
		void warn(LogEvent::ptr event);
		void error(LogEvent::ptr event);
		void fatal(LogEvent::ptr event);

		void addAppender(LogAppender::ptr appenders);
		void delAppender(LogAppender::ptr appenders);
		LogLevel::Level getLevel() const { return m_level; }
		void setLevel(LogLevel::Level val) { m_level = val; }

	private:
		std::string m_name;						 // 日志名称
		LogLevel::Level m_level;				 // 日志级别
		std::list<LogAppender::ptr> m_appenders; // Appender列表
	};

	// 输出到控制台
	class StdoutLogAppender : public LogAppender
	{
	public:
		typedef std::shared_ptr<StdoutLogAppender> ptr;
		virtual void log(LogLevel::Level level, LogEvent::ptr event) override;

	private:
	};

	// 输出到文件的Appender
	class FileLogAppender : public LogAppender
	{
	public:
		typedef std::shared_ptr<FileLogAppender> ptr;
		FileLogAppender(const std::string &filename);
		virtual void log(LogLevel::Level level, LogEvent::ptr event) override;

		bool reopen(); // 文件打开成功返回true

	private:
		std::string m_fileName;
		std::ofstream m_filestream;
	};

} // namespace sylar

#endif
