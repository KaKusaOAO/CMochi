//
//  common.hpp
//  MiaCrate++
//
//  Created by 咔咔 on 2023/12/6.
//

#if defined(__cplusplus)
#ifndef __MC_COMMON_HPP_HEADER_GUARD
#define __MC_COMMON_HPP_HEADER_GUARD

#include <cstdint>
#include <stdexcept>
#include <source_location>

#define __MC_REF_TYPE(name) std::shared_ptr< name >

namespace Mochi {

typedef bool Bool;

typedef uint8_t  UInt8;
typedef uint16_t UInt16;
typedef uint32_t UInt32;
typedef uint64_t UInt64;

typedef int8_t  Int8;
typedef int16_t Int16;
typedef int32_t Int32;
typedef int64_t Int64;

[[noreturn]] void ThrowNotImplemented(const std::source_location loc = std::source_location::current());

}
#endif /* common_hpp */
#endif // defined(__cplusplus)
