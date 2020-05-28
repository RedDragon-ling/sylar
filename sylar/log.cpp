#include "log.h"
#include <map>
#include <functional>

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
            XX(DEBUG); // 替换为case LogLevel::DEBUG: return "DEBUG";break;
            XX(INFO);  // 同理
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
                i->log(std::shared_ptr<Logger>(this), level, event);
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
    void FileLogAppender::log(std::shared_ptr<Logger> ptr, LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
            m_filestream << m_formatter->format(ptr, level, event);
    }

    bool FileLogAppender::reopen()
    {
        if (m_filestream)
            m_filestream.close();
        m_filestream.open(m_fileName);
        return !m_filestream;
    }
    // StdoutLogAppender的实现
    void StdoutLogAppender::log(std::shared_ptr<Logger> ptr, LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
            std::cout << m_formatter->format(ptr, level, event);
    };

    LogFormatter::LogFormatter(const std::string &pattern)
        : m_pattern(pattern)
    {
    }
    std::string LogFormatter::format(std::shared_ptr<Logger> ptr, LogLevel::Level level, LogEvent::ptr event)
    {
        std::stringstream ss; //没见过
        for (auto &i : m_items)
        {
            i->format(ss, ptr, level, event);
        }
        return ss.str();
    }

    void LogFormatter::init()
    {
        // tuple是元组，可以存不同的数据类型
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

        static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_items =
        {
#define XX(str, C) \
    {#str, [](const std::string &fmt) { return FormatItem::ptr(new C(fmt)); }

            XX(m, MessageFormatItem),    // %m -- 消息体
            XX(p, LevelFormatItem),      // %p -- level
            XX(r, ElapseFormatItem),     // %r -- 启动后的时间
            XX(c, LoggerNameFormatItem), // %c -- 日志名
            XX(t, ThreadIdFormatItem),   // %t -- 线程id
            XX(n, NewLineFormatItem),    // %n -- 回车换行
            XX(d, DateTimeFormatItem),   // %d -- 时间
            XX(f, FileNameFormatItem),   // %f -- 文件名
            XX(l, LineFormatItem)        // %l -- 行号
#undef XX
        };

        for (auto &i : vec)
        {
            if (std::get<2>(i) == 0) //get<2>(i)就是读tuple的第2个数据
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            else
            {
                auto it = s_format_items.find(std::get<0>(i));
                if (it == s_format_items.end())
                    m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                else
                    m_items.push_back(it->second(std::get<1>(i)));
            }
            std::cout << std::get<0>(i) << " - " << std::get<1>(i) << " - " << std::get<2>(i) << std::endl;
        }
    };
    // 日志描述
    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        virtual void format(std::ostream &os, std::shared_ptr<Logger> ptr, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getContent();
        }
    };
    // 日志级别
    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        virtual void format(std::ostream &os, std::shared_ptr<Logger> ptr, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << LogLevel::ToString(level);
        }
    };
    // 日志启动时间（ms）
    class ElapseFormatItem : public LogFormatter::FormatItem
    {
    public:
        virtual void format(std::ostream &os, std::shared_ptr<Logger> ptr, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getElapse();
        }
    };
    // 日志名
    class LoggerNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        virtual void format(std::ostream &os, std::shared_ptr<Logger> ptr, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << ptr->getName();
        }
    };
    // 线程ID
    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        virtual void format(std::ostream &os, std::shared_ptr<Logger> ptr, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getThreadId();
        }
    };
    // 协程ID
    class FiberIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        virtual void format(std::ostream &os, std::shared_ptr<Logger> ptr, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getFiberId();
        }
    };
    // 日期时间
    class DateTimeFormatItem : public LogFormatter::FormatItem
    {
    public:
        DateTimeFormatItem(const std::string &format = "%Y:%m:%d %H:%M:%S")
            : m_format(format) {}
        virtual void format(std::ostream &os, std::shared_ptr<Logger> ptr, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getTime();
        }

    private:
        std::string m_format; // 时间的格式
    };
    // 文件名
    class FileNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        virtual void format(std::ostream &os, std::shared_ptr<Logger> ptr, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getFile();
        }
    };
    // 行号
    class LineFormatItem : public LogFormatter::FormatItem
    {
    public:
        virtual void format(std::ostream &os, std::shared_ptr<Logger> ptr, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getLine();
        }
    };
    // 换行
    class NewLineFormatItem : public LogFormatter::FormatItem
    {
    public:
        virtual void format(std::ostream &os, std::shared_ptr<Logger> ptr, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << std::endl;
        }
    };
    //
    class StringFormatItem : public LogFormatter::FormatItem
    {
    public:
        StringFormatItem(const std::string &str)
            : LogFormatter::FormatItem(str), m_string(str) {}
        virtual void format(std::ostream &os, std::shared_ptr<Logger> ptr, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << m_string;
        }

    private:
        std::string m_string;
    };
} // namespace sylar
