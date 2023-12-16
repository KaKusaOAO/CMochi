//
//  logging.hpp
//  MiaCrate++
//
//  Created by 咔咔 on 2023/12/6.
//

#if defined(__cplusplus)
#ifndef __MC_LOGGING_HPP_HEADER_GUARD
#define __MC_LOGGING_HPP_HEADER_GUARD

#include <Mochi/components.hpp>
#include <ctime>
#include <iostream>
#include <chrono>

namespace Mochi {

    enum class LogLevel {
        Verbose, Log, Info, Warn, Error, Fatal
    };

    std::string GetLogLevelName(LogLevel level);
    bool GetLogLevelName(LogLevel level, std::string *outName);

    struct LoggerEventArgs {
        LogLevel level;
        std::shared_ptr<IComponent> content;
        std::shared_ptr<IComponent> tag;
        std::shared_ptr<TextColor> color;
        std::thread::id threadId;
        std::chrono::time_point<std::chrono::system_clock> timestamp;
    };

    class IAsyncLogEventDelegate {
        using Signature = std::function<std::future<void>(std::shared_ptr<LoggerEventArgs>)>;;
        
    public:
        virtual std::future<void> Invoke(std::shared_ptr<LoggerEventArgs> ev) = 0;
        static std::shared_ptr<IAsyncLogEventDelegate> Create(Signature delegate);
    };

    class Logger {
        using Handler = AsyncEventHandler<IAsyncLogEventDelegate>;
        using HandlerRef = std::unique_ptr<Handler>;
        using RecordCall = std::queue<std::function<void()>>;

    private:
        static HandlerRef _loggedHandler;
        static RecordCall _recordCall;
        static std::mutex _recordCallMutex;
        static Bool _isInitialized;
        static Bool _bootstrapped;
        static Bool _isRunning;
        static std::thread::id _threadId;
        static std::shared_ptr<std::thread> _thread;
        
        static void RunEventLoop();
        static void CallOrQueue(std::function<void()> action);
        static void InternalOnLogged(std::shared_ptr<LoggerEventArgs> data);
        static void Log(LogLevel level,
                        std::shared_ptr<IComponent> text,
                        std::shared_ptr<TextColor> color,
                        std::shared_ptr<IComponent> name);

    public:
        static void Init();
        static void AddLoggedListener(std::shared_ptr<IAsyncLogEventDelegate> delegate);
        static void RemoveLoggedListener(std::shared_ptr<IAsyncLogEventDelegate> delegate);
        static void RunThreaded();
        static void RunManualPoll();
        static void RunBlocking();
        static void Join();
        static void PollEvents();
        static void Info(std::string str, std::string name = "Logger");
        static void Warn(std::string str, std::string name = "Logger");
        static void Error(std::string str, std::string name = "Logger");
    };

    class NamedLogger {
    private:
        std::string _name;
    public:
        NamedLogger(std::string name);
        
        void Info(std::string str);
        void Warn(std::string str);
        void Error(std::string str);
    };

}

#endif /* logging_h */
#endif
