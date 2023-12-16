//
//  core.hpp
//  MiaCrate++
//
//  Created by 咔咔 on 2023/12/6.
//

#if defined(__cplusplus)
#ifndef __MC_CORE_HPP_HEADER_GUARD
#define __MC_CORE_HPP_HEADER_GUARD

#include <Mochi/macros.hpp>
#include <memory>
#include <cstdint>
#include <stdexcept>
#include <source_location>

namespace __MC_NAMESPACE {

    using Bool = bool;
    constexpr Bool True  = true;
    constexpr Bool False = false;

    using Int8  = int8_t;
    using Int16 = int16_t;
    using Int32 = int32_t;
    using Int64 = int64_t;

    using UInt8  = uint8_t;
    using UInt16 = uint16_t;
    using UInt32 = uint32_t;
    using UInt64 = uint64_t;

    template <class T, class TBase>
    concept IsDerived = std::is_base_of<TBase, T>::value;

    template <Bool B>
    concept IsTrue = B;
    
	template <int A, int B>
	concept IsGreaterThan = A > B;

    template <typename T>
    using Handle = std::shared_ptr<T>;

    [[noreturn]] void ThrowNotImplemented(const std::source_location loc = std::source_location::current());

}


#endif /* core_hpp */
#endif
