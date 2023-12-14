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
    IComponent& content;
    IComponent& tag;
    TextColor& color;
    std::thread::id threadId;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
};

class IAsyncLogEventDelegate {
public:
    using Signature = std::function<std::future<void>(LoggerEventArgs&)>;
    
    virtual std::future<void> Invoke(LoggerEventArgs& ev) = 0;
    static IAsyncLogEventDelegate& Create(Signature delegate);
};

class Logger {
    using Handler = AsyncEventHandler<IAsyncLogEventDelegate>;
    using RecordCall = std::queue<std::function<void()>>;

private:
    static Handler& _loggedHandler;
    static RecordCall _recordCall;
    static std::mutex _recordCallMutex;
    static Bool _isInitialized;
    static Bool _bootstrapped;
    static Bool _isRunning;
    static std::thread::id _threadId;
    static std::shared_ptr<std::thread> _thread;
    
    static void RunEventLoop();
    static void CallOrQueue(std::function<void()> action);
    static void InternalOnLogged(LoggerEventArgs& data);
    static void Log(LogLevel level,
                    IComponent& text,
                    TextColor& color,
                    IComponent& name);

public:
    static void Init();
    static void AddLoggedListener(IAsyncLogEventDelegate& delegate);
    static void RemoveLoggedListener(IAsyncLogEventDelegate& delegate);
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
