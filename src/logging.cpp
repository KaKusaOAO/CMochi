//
//  logging.cpp
//  MiaCrate++
//
//  Created by 咔咔 on 2023/12/7.
//

#include <Mochi/logging.hpp>

namespace __MC_NAMESPACE {

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

    std::shared_ptr<IAsyncLogEventDelegate> IAsyncLogEventDelegate::Create(IAsyncLogEventDelegate::Signature delegate) {
        class Instance : public IAsyncLogEventDelegate {
        private:
            IAsyncLogEventDelegate::Signature _d;
        public:
            Instance(IAsyncLogEventDelegate::Signature d) : _d(d) {
                
            }
            
            std::future<void> Invoke(std::shared_ptr<LoggerEventArgs> ev) override {
                return _d(ev);
            }
        };
        
        return std::make_shared<Instance>(delegate);
    }

    // MARK: -

    Logger::HandlerRef Logger::_loggedHandler = std::make_unique<Logger::Handler>();
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
            
            InternalOnLogged(std::make_shared<LoggerEventArgs>(LoggerEventArgs{
                LogLevel::Warn,
                Component::Literal("*** Logger is not bootstrapped. ***"),
                Component::Literal("Logger"),
                TextColor::Gold,
                std::this_thread::get_id(),
                current
            }));
            
            InternalOnLogged(std::make_shared<LoggerEventArgs>(LoggerEventArgs{
                LogLevel::Warn,
                Component::Literal("Logger now requires either RunThreaded(), RunBlocking() or RunManualPoll() to poll log events."),
                Component::Literal("Logger"),
                TextColor::Gold,
                std::this_thread::get_id(),
                current
            }));
            
            InternalOnLogged(std::make_shared<LoggerEventArgs>(LoggerEventArgs {
                LogLevel::Warn,
                Component::Literal("The threaded approach will be used by default."),
                Component::Literal("Logger"),
                TextColor::Gold,
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

    void Logger::InternalOnLogged(std::shared_ptr<LoggerEventArgs> data) {
        for (Logger::Handler::HandlerEntry handler : _loggedHandler->GetHandlers()) {
            try {
                handler->Invoke(data);
            } catch (std::exception &ex) {
                std::cout << "Exception: " << ex.what() << "\n";
            }
        }
    }

    void Logger::Log(LogLevel level,
                    std::shared_ptr<IComponent> text,
                    std::shared_ptr<TextColor> color,
                    std::shared_ptr<IComponent> name) {
        auto threadId = std::this_thread::get_id();
        auto tClone = text->Clone();
        auto nameClone = name->Clone();
        
        auto args = std::make_shared<LoggerEventArgs>();
        args->level = level;
        args->tag = nameClone;
        args->content = tClone;
        args->color = color;
        args->threadId = threadId;
        args->timestamp = std::chrono::system_clock::now();
        
        CallOrQueue([args]() {
            InternalOnLogged(args);
        });
    }

    void Logger::Init() {
        if (_isInitialized) {
            throw std::runtime_error("Logger already initialized.");
        }
        
        _isInitialized = true;
        _loggedHandler = std::make_unique<AsyncEventHandler<IAsyncLogEventDelegate>>();
    }

    void Logger::AddLoggedListener(std::shared_ptr<IAsyncLogEventDelegate> delegate) {
        if (!_loggedHandler) {
            throw std::runtime_error("_loggedHandler is not initialized");
        }
        
        _loggedHandler->AddHandler(delegate);
    }

    void Logger::RemoveLoggedListener(std::shared_ptr<IAsyncLogEventDelegate> delegate) {
        if (!_loggedHandler) {
            throw std::runtime_error("_loggedHandler is not initialized");
        }
        
        _loggedHandler->RemoveHandler(delegate);
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
    
    std::future<void> Logger::FlushAsync() {
        // Wrapping a promise in a shared_ptr and then move it.
        // It works?
        auto promise = std::make_shared<std::promise<void>>();
        std::future<void> future = promise->get_future();
        
        if (std::this_thread::get_id() == _threadId) {
            // Don't run this on logger thread
            promise->set_value();
            return future;
        }
        
        // Inject a hook to inform that previous events are handled
        auto lock = std::lock_guard(_recordCallMutex);
        _recordCall.push([promise = std::move(promise)]() {
            // Complete the promise on executed
            promise->set_value();
        });

        return future; 
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
        Log(LogLevel::Info, Component::Literal(str), TextColor::Green, Component::Literal(name));
    }

    void Logger::Warn(std::string str, std::string name) {
        Log(LogLevel::Warn, Component::Literal(str), TextColor::Gold, Component::Literal(name));
    }

    void Logger::Error(std::string str, std::string name) {
        Log(LogLevel::Error, Component::Literal(str), TextColor::Red, Component::Literal(name));
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
