//
//  common.cpp
//  MiaCrate++
//
//  Created by 咔咔 on 2023/12/8.
//

#include <Mochi/core.hpp>
#include <sstream>

namespace __MC_NAMESPACE {

    MOCHI_NORETURN void ThrowNotImplemented(
#if defined(MOCHI_CPLUSPLUS_HAS_CXX20)
        const std::source_location loc
#endif // defined(MOCHI_CPLUSPLUS_HAS_CXX20)
    ) {
        class NotImplementedException : public std::exception {
        private:
            std::string _message;
        
        public:
            NotImplementedException(std::string message) : _message(message) {}
            const char * what() const noexcept override { return _message.c_str(); }
        };
    
        std::stringstream str;

#if defined(MOCHI_CPLUSPLUS_HAS_CXX20)
        str << loc.file_name() << " (" << loc.line() << ":" << loc.column() << ") ";
        str << "In '" << loc.function_name() << "()' -> Not implemented";
#else
        str << "Not implemented";
#endif

        throw NotImplementedException(str.str());
    }

}
