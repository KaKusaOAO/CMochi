//
// Created by 咔咔 on 2023/12/22.
//

#include <Mochi/Logging.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <array>
#include <functional>

template <typename T>
using MHandle          = ::MOCHI_NAMESPACE::Handle<T>;
using MLoggerEventArgs = ::MOCHI_NAMESPACE::LoggerEventArgs;

std::future<void> OnLogged(MHandle<MLoggerEventArgs> ev) {
    auto timeT = std::chrono::system_clock::to_time_t(ev->timestamp);
    auto time = localtime(&timeT);

    std::stringstream sb;

    sb << (time->tm_year + 1900) << "-";
    sb << std::setfill('0') << std::setw(2);
    sb << time->tm_mon << "-";
    sb << std::setfill('0') << std::setw(2);
    sb << time->tm_mday << " ";

    sb << std::setfill('0') << std::setw(2);
    sb << time->tm_hour << ":";
    sb << std::setfill('0') << std::setw(2);
    sb << time->tm_min << ":";
    sb << std::setfill('0') << std::setw(2);
    sb << time->tm_sec << " ";

    sb << "[Thread@" << ev->threadId << "] ";

    std::string levelName;

    if (MOCHI_NAMESPACE::GetLogLevelName(ev->level, &levelName)) {
        std::transform(levelName.begin(), levelName.end(), levelName.begin(), ::toupper);
    } else {
        levelName = "<unknown>";
    }

    sb << "[" << levelName << "] ";

    sb << "[";

    using MLiteralContent = ::MOCHI_NAMESPACE::LiteralContent;

    if (auto tag = std::dynamic_pointer_cast<MLiteralContent>(ev->tag->GetContent())) {
        sb << tag->text;
    }
    else {
        sb << "???";
    }

    sb << "] ";

    if (auto literal = std::dynamic_pointer_cast<MLiteralContent>(ev->content->GetContent())) {
        sb << literal->text;
    }
    else {
        sb << "*** Invalid content type at " << ev->content->GetContent();
    }

    sb << "\n";
    std::cout << sb.str();

    std::promise<void> promise;
    promise.set_value();
    return promise.get_future();
}


int main(int argc, char** argv) {
    using MLogger                 = ::MOCHI_NAMESPACE::Logger;
    using MIAsyncLogEventDelegate = ::MOCHI_NAMESPACE::IAsyncLogEventDelegate;

    MLogger::Init();
    MLogger::AddLoggedListener(MIAsyncLogEventDelegate::Create(OnLogged));
    MLogger::RunThreaded();

    MLogger::Info("Hello, world.");
    MLogger::Warn("This is a warning!");

    MLogger::FlushAsync().wait();
    return 0;
}