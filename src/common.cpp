//
//  common.cpp
//  MiaCrate++
//
//  Created by 咔咔 on 2023/12/8.
//

#include <Mochi/common.hpp>
#include <Mochi/core.hpp>

namespace Mochi {

[[noreturn]] void ThrowNotImplemented(const std::source_location loc) {
    class NotImplementedException : public std::exception {
    private:
        std::string _message;
        
    public:
        NotImplementedException(std::string message) : _message(message) {}
        const char * what() const noexcept override { return _message.c_str(); }
    };
    
    std::stringstream str;
    str << loc.file_name() << " (" << loc.line() << ":" << loc.column() << ") ";
    str << "In '" << loc.function_name() << "()' -> Not implemented";
    
    throw NotImplementedException(str.str());
}

}
