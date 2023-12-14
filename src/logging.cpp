//
//  logging.cpp
//  MiaCrate++
//
//  Created by 咔咔 on 2023/12/7.
//

#include <Mochi/logging.hpp>

namespace Mochi {

struct LogLevelLookupEntry {
    LogLevel level;
    std::string name;
};

// MARK: LogLevelLookupEntry LogLevelTable[]
LogLevelLookupEntry LogLevelTable[] = {
    {LogLevel::Verbose, "Verbose"},
    {LogLevel::Log, "Log"},
    {LogLevel::Info, "Info"},
    {LogLevel::Warn, "Warn"},
    {LogLevel::Error, "Error"},
    {LogLevel::Fatal, "Fatal"},
};

std::string GetLogLevelName(LogLevel level) {
    std::string result;
    
    if (GetLogLevelName(level, &result)) {
        return result;
    }
    
    return "<unknown>";
}

bool GetLogLevelName(LogLevel level, std::string *outName) {
    int count = sizeof(LogLevelTable) / sizeof(LogLevelLookupEntry);
    if ((int) level < 0 || (int) level >= count) return false;

    if (outName) *outName = LogLevelTable[(int) level].name;
    return true;
    
//    for (int i = 0; i < count; i++) {
//        auto entry = LogLevelTable[i];
//        if (entry.level == level) {
//            if (outName) *outName = entry.name;
//            return true;
//        }
//    }
//    
//    return false;
}

// MARK: -

IAsyncLogEventDelegate& IAsyncLogEventDelegate::Create(IAsyncLogEventDelegate::Signature delegate) {
    class Instance : public IAsyncLogEventDelegate {
    private:
        IAsyncLogEventDelegate::Signature _d;
    public:
        Instance(IAsyncLogEventDelegate::Signature d) : _d(d) {
            
        }
        
        std::future<void> Invoke(LoggerEventArgs& ev) override {
            return _d(ev);
        }
    };
    
    return CreateReference<Instance>(delegate);
}

// MARK: -

Logger::Handler& Logger::_loggedHandler = CreateReference<Logger::Handler>();
Logger::RecordCall Logger::_recordCall = std::queue<std::function<void()>>();
std::mutex Logger::_recordCallMutex = std::mutex();
Bool Logger::_bootstrapped = false;
Bool Logger::_isRunning = false;
Bool Logger::_isInitialized = false;
std::thread::id Logger::_threadId = std::this_thread::get_id();
std::shared_ptr<std::thread> Logger::_thread = std::shared_ptr<std::thread>();

void Logger::RunEventLoop() {
    _isRunning = true;
    
    while (_isRunning) {
        while (_isRunning && _recordCall.empty()) {
            std::this_thread::yield();
        }
        
        PollEvents();
    }
    
    // Flush all events before exiting the event loop
    PollEvents();
}

void Logger::CallOrQueue(std::function<void()> action) {
    if (!_bootstrapped) {
        RunThreaded();
        
        auto current = std::chrono::system_clock::now();
        
        InternalOnLogged(CreateReference<LoggerEventArgs>(LoggerEventArgs{
            LogLevel::Warn,
            Component::Literal("*** Logger is not bootstrapped. ***"),
            Component::Literal("Logger"),
            (TextColor&) TextColor::Gold,
            std::this_thread::get_id(),
            current
        }));
        
        InternalOnLogged(CreateReference<LoggerEventArgs>(LoggerEventArgs{
            LogLevel::Warn,
            Component::Literal("Logger now requires either RunThreaded(), RunBlocking() or RunManualPoll() to poll log events."),
            Component::Literal("Logger"),
            (TextColor&) TextColor::Gold,
            std::this_thread::get_id(),
            current
        }));
        
        InternalOnLogged(CreateReference<LoggerEventArgs>(LoggerEventArgs {
            LogLevel::Warn,
            Component::Literal("The threaded approach will be used by default."),
            Component::Literal("Logger"),
            (TextColor&) TextColor::Gold,
            std::this_thread::get_id(),
            current
        }));
    }
    
    if (!_bootstrapped) {
        throw std::runtime_error("Logger is not bootstrapped.");
    }
    
    std::lock_guard<std::mutex> lock(_recordCallMutex);
    if (std::this_thread::get_id() != _threadId) {
        _recordCall.push(action);
    } else {
        action();
    }
}

void Logger::InternalOnLogged(LoggerEventArgs& data) {
    for (auto& handler : _loggedHandler.GetHandlers()) {
        try {
            handler.get().Invoke(data);
        } catch (std::exception &ex) {
            std::cout << "Exception: " << ex.what() << "\n";
        }
    }
}

void Logger::Log(LogLevel level,
                 IComponent& text,
                 TextColor& color,
                 IComponent& name) {
    auto threadId = std::this_thread::get_id();
    auto& tClone = text.Clone();
    auto& nameClone = name.Clone();
    
    auto& args = CreateReference<LoggerEventArgs>(LoggerEventArgs{
        level,
        tClone,
        nameClone,
        color,
        threadId,
        std::chrono::system_clock::now()
    });
    
    CallOrQueue([&args]() {
        InternalOnLogged(args);
    });
}

void Logger::Init() {
    if (_isInitialized) {
        throw std::runtime_error("Logger already initialized.");
    }
    
    _isInitialized = true;
    _loggedHandler = CreateReference<AsyncEventHandler<IAsyncLogEventDelegate>>();
}

void Logger::AddLoggedListener(IAsyncLogEventDelegate& delegate) {
    // if (!_loggedHandler) {
    //     throw std::runtime_error("_loggedHandler is not initialized");
    // }
    
    _loggedHandler.AddHandler(delegate);
}

void Logger::RemoveLoggedListener(IAsyncLogEventDelegate& delegate) {
    // if (!_loggedHandler) {
    //     throw std::runtime_error("_loggedHandler is not initialized");
    // }
    
    _loggedHandler.RemoveHandler(delegate);
}

void Logger::RunThreaded() {
    if (_bootstrapped) return;
    _bootstrapped = true;
    
    _thread = std::make_shared<std::thread>([&]() {
        _threadId = std::this_thread::get_id();
        RunEventLoop();
    });
}

void Logger::RunManualPoll() {
    if (_bootstrapped) return;
    _bootstrapped = true;
    _threadId = std::this_thread::get_id();
}

void Logger::RunBlocking() {
    if (_bootstrapped) return;
    _bootstrapped = true;
    _threadId = std::this_thread::get_id();
    RunEventLoop();
}

void Logger::Join() {
    _isRunning = false;
    
    if (_thread.get()) {
        _thread->join();
    }
}

void Logger::PollEvents() {
    if (!_bootstrapped) {
        throw std::runtime_error("Logger is not bootstrapped");
    }
    
    if (_threadId != std::this_thread::get_id()) {
        throw std::runtime_error("PollEvents() called from wrong thread");
    }
    
    std::lock_guard<std::mutex> lock(_recordCallMutex);
    while (!_recordCall.empty()) {
        auto result = _recordCall.front();
        
        try {
            result();
        } catch (std::exception &ex) {
            // Ignored.
        }
        
        _recordCall.pop();
        std::this_thread::yield();
    }
}

void Logger::Info(std::string str, std::string name) {
    Log(LogLevel::Info, Component::Literal(str), (TextColor&) TextColor::Green, Component::Literal(name));
}

void Logger::Warn(std::string str, std::string name) {
    Log(LogLevel::Warn, Component::Literal(str), (TextColor&) TextColor::Gold, Component::Literal(name));
}

void Logger::Error(std::string str, std::string name) {
    Log(LogLevel::Error, Component::Literal(str), (TextColor&) TextColor::Red, Component::Literal(name));
}

// MARK: -

NamedLogger::NamedLogger(std::string name) : _name(name) {}

void NamedLogger::Info(std::string str) {
    Logger::Info(str, _name);
}

void NamedLogger::Warn(std::string str) {
    Logger::Warn(str, _name);
}

void NamedLogger::Error(std::string str) {
    Logger::Error(str, _name);
}

}
