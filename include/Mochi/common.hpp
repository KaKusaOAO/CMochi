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

namespace Mochi {

using Bool = bool;

using UInt8  = uint8_t;
using UInt16 = uint16_t;
using UInt32 = uint32_t;
using UInt64 = uint64_t;

using Int8  = int8_t;
using Int16 = int16_t;
using Int32 = int32_t;
using Int64 = int64_t;

[[noreturn]] void ThrowNotImplemented(const std::source_location loc = std::source_location::current());

}
#endif /* common_hpp */
#endif // defined(__cplusplus)
