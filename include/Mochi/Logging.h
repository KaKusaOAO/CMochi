//
//  logging.hpp
//  MiaCrate++
//
//  Created by 咔咔 on 2023/12/6.
//

#if defined(__cplusplus)
#ifndef __MOCHI_LOGGING_H_HEADER_GUARD
#define __MOCHI_LOGGING_H_HEADER_GUARD

#include <Mochi/Components.h>
#include <ctime>
#include <iostream>
#include <chrono>

namespace MOCHI_NAMESPACE {

    enum class LogLevel {
        Verbose, Log, Info, Warn, Error, Fatal
    };

    std::string GetLogLevelName(LogLevel level);
    bool GetLogLevelName(LogLevel level, std::string *outName);

    struct LoggerEventArgs {
        LogLevel level;
        Handle<IComponent> content;
        Handle<IComponent> tag;
        Handle<TextColor> color;
        std::thread::id threadId;
        std::chrono::time_point<std::chrono::system_clock> timestamp;
    };

    class IAsyncLogEventDelegate {
        using Signature = std::function<std::future<void>(Handle<LoggerEventArgs>)>;;
        
    public:
        virtual std::future<void> Invoke(Handle<LoggerEventArgs> ev) = 0;
        static Handle<IAsyncLogEventDelegate> Create(Signature delegate);
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
        static Handle<std::thread> _thread;
        
        static void RunEventLoop();
        static void CallOrQueue(std::function<void()> action);
        static void InternalOnLogged(Handle<LoggerEventArgs> data);
        static void Log(LogLevel level,
                        Handle<IComponent> text,
                        Handle<TextColor> color,
                        Handle<IComponent> name);

    public:
        static void Init();
        static void AddLoggedListener(Handle<IAsyncLogEventDelegate> delegate);
        static void RemoveLoggedListener(Handle<IAsyncLogEventDelegate> delegate);
        static void RunThreaded();
        static void RunManualPoll();
        static void RunBlocking();
        static void Join();
        static void PollEvents();
        static std::future<void> FlushAsync();
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
