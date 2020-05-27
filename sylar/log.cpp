#include "log.h"

namespace sylar
{
    //LogLevel类的实现
    const char *LogLevel::ToString(LogLevel::Level level)
    {
        switch (level)
        {
#define XX(name)         \
    case LogLevel::name: \
        return #name;    \
        break;
            XX(DEBUG);
            XX(INFO);
            XX(WARN);
            XX(ERROR);
            XX(FATAL);
#undef XX
        default:
            return "UNKNOW";
        }
        return "UNKNOW";
    }

    // Logger类的实现
    Logger::Logger(const std::string &name)
        : m_name(name) {}
    void Logger::addAppender(LogAppender::ptr appender)
    {
        m_appenders.push_back(appender);
    }
    void Logger::delAppender(LogAppender::ptr appender)
    {
        for (auto i = m_appenders.begin(); i != m_appenders.end(); i++)
        {
            if (*i == appender)
            {
                m_appenders.erase(i);
                break;
            }
        }
    }
    void Logger::log(LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            for (auto &i : m_appenders)
            {
                i->log(level, event);
            }
        }
    }
    void Logger::debug(LogEvent::ptr event)
    {
        log(LogLevel::DEBUG, event);
    }
    void Logger::info(LogEvent::ptr event)
    {
        log(LogLevel::INFO, event);
    }
    void Logger::warn(LogEvent::ptr event)
    {
        log(LogLevel::WARN, event);
    }
    void Logger::error(LogEvent::ptr event)
    {
        log(LogLevel::ERROR, event);
    }
    void Logger::fatal(LogEvent::ptr event)
    {
        log(LogLevel::FATAL, event);
    };

    // FileLogAppender的实现
    void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
            m_filestream << m_formatter->format(level, event);
    }

    bool FileLogAppender::reopen()
    {
        if (m_filestream)
            m_filestream.close();
        m_filestream.open(m_fileName);
        return !m_filestream;
    }
    // StdoutLogAppender的实现
    void StdoutLogAppender::log(LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
            std::cout << m_formatter->format(level, event);
    };

    LogFormatter::LogFormatter(const std::string &pattern)
        : m_pattern(pattern)
    {
    }
    std::string LogFormatter::format(LogLevel::Level level, LogEvent::ptr event)
    {
        std::stringstream ss; //没见过
        for (auto &i : m_items)
        {
            i->format(ss, level, event);
        }
        return ss.str();
    }

    void LogFormatter::init() // 没这个函数
    {
        std::vector<std::tuple<std::string, std::string, int>> vec;
        size_t last_pos = 0;
        std::string nstr;
        for (size_t i = 0; i < m_pattern.size(); ++i)
        {
            if (m_pattern[i] != '%')
            {
                nstr.append(1, m_pattern[i]);
                continue;
            }

            if ((i + 1) < m_pattern.size())
            {
                if (m_pattern[i + 1] == '%')
                {
                    nstr.append(1, '%');
                    continue;
                }
            }

            size_t n = i + 1, fmt_begin = 0;
            int fmt_status = 0;

            std::string str, fmt;
            while (n < m_pattern.size())
            {
                if (isspace(m_pattern[n]))
                    break;
                if (fmt_status == 0)
                {
                    if (m_pattern[n] == '{')
                    {
                        str = m_pattern.substr(i + 1, n - i - 1);
                        fmt_status = 1; // 解析格式
                        fmt_begin = n;
                        ++n;
                        continue;
                    }
                }
                if (fmt_status == 1)
                {
                    if (m_pattern[n] == '}')
                    {
                        fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                        fmt_status = 2;
                        break;
                    }
                }
            }
            if (fmt_status == 0)
            {
                if (!nstr.empty())
                    vec.push_back(std::make_tuple(str, fmt, 1));
                str = m_pattern.substr(i + 1, n - i - 1);
                vec.push_back(std::make_tuple(str, fmt, 1));
                i = n;
            }
            else if (fmt_status == 1)
            {
                std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            }
            else if (fmt_status == 2)
            {
                if (!nstr.empty())
                    vec.push_back(std::make_tuple(nstr, "", 0));
                vec.push_back(std::make_tuple(str, fmt, 1));
                i = n;
            }
        }
        if (!nstr.empty())
            vec.push_back(std::make_tuple(nstr, "", 0));

        // %m -- 消息体
        // %p -- level
        // %r -- 启动后的时间
        // %c -- 日志名称
        // %t -- 线程id
        // %n -- 回车换行
        // %d -- 时间
        // %f -- 文件名
        // %l -- 行号
    }

    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        virtual void format(std::ostream &os, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        virtual void format(std::ostream &os, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << LogLevel::ToString(level);
        }
    };
    const char *m_file = nullptr; // 文件名
    int32_t m_line = 0;           // 行号
    uint32_t m_elapse = 0;        // 程序启动到现在的毫秒数
    uint32_t m_threadId = 0;      // 线程ID
    uint32_t m_fiberId = 0;       // 协程ID
    uint32_t m_time;              // 时间戳
} // namespace sylar
