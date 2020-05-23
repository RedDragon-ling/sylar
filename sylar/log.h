#ifndef _SYLAY_LOG_H_
#define _SYLAY_LOG_H_

#include <string>
#include <stdint.h>
#include <memory>

namespace sylar{

class LogEvent{
public:
	typedef std::shared_ptr<LogEvent> ptr;
	LogEvent();
private:
	const char* m_file = nullptr;	// 文件名
	int32_t m_line = 0;				// 行号
	uint32_t m_elapse = 0;			// 程序启动到现在的毫秒数
	uint32_t m_threadId = 0;		// 线程ID
	uint32_t n_fiberId = 0;			// 协程ID
	uint32_t m_time;				// 时间戳
	std::string m_cotent;			// 消息

};
// 日志级别
class LogLevel{
	enum Level{
		DEBUG = 1,
		INFO = 2,
		WARN = 3,
		ERROR = 4,
		FATAL = 5,					// 致命
	};
};

// 日志格式器
class LogFormatter{
public:
	typedef std::shared_ptr<LogFormatter> ptr;

	std::string format(LogEvent::ptr event);
private:
};

// 日志输出地
class LogAppender{
public:
	typedef std::shared_ptr<LogAppender> ptr;
	virtual ~LogAppender(){}
	
	void log(LogLevel::Level level, LogEvent::ptr event);
private:
	LogLevel::Level m_level;
};


// 日志器
class Logger{
public:
	typedef std::shared_ptr<Logger> ptr;

	Logger(const std::string& name = "root");
	void log(LogLevel::level level, LogEvent::ptr event);
private:
	std::string m_name;
	LogLevel::level m_level;
	LogAppender::ptr;
};

// 输出到控制台
class StdoutLogAppender : public LogAppender{

};

// 输出到文件的Appender
class FileLogAppender : public LogAppender{

};

}

#endif
