#pragma once

#ifndef __MC_MACROS_HPP_HEADER_GUARD
#define __MC_MACROS_HPP_HEADER_GUARD

#if defined(__cplusplus)

#define __MC_NAMESPACE Mochi

#if defined(_MSVC_LANG) 
#   define MOCHI_CPLUSPLUS_VERSION _MSVC_LANG
#else   // defined(_MSVC_LANG) 
#   define MOCHI_CPLUSPLUS_VERSION __cplusplus
#endif  // defined(_MSVC_LANG)

#if MOCHI_CPLUSPLUS_VERSION >= 202002L
#   define MOCHI_CPLUSPLUS_HAS_CXX20
#endif // MOCHI_CPLUSPLUS_VERSION >= 202002L

#if MOCHI_CPLUSPLUS_VERSION >= 201703L
#   define MOCHI_CPLUSPLUS_HAS_CXX17
#endif // MOCHI_CPLUSPLUS_VERSION >= 201703L

#if MOCHI_CPLUSPLUS_VERSION >= 201402L
#   define MOCHI_CPLUSPLUS_HAS_CXX14
#endif // MOCHI_CPLUSPLUS_VERSION >= 201402L

#if MOCHI_CPLUSPLUS_VERSION >= 201103L
#   define MOCHI_CPLUSPLUS_HAS_CXX11
#endif // MOCHI_CPLUSPLUS_VERSION >= 201103L

#if __has_cpp_attribute(deprecated)
#   define MOCHI_DEPRECATED(message) [[deprecated(message)]]
#else
#   define MOCHI_DEPRECATED(message)
#endif // __has_cpp_attribute(deprecated)

#if __has_cpp_attribute(noreturn)
#   define MOCHI_NORETURN [[noreturn]]
#else
#   define MOCHI_NORETURN
#endif // __has_cpp_attribute(noreturn)

#if __has_cpp_attribute(assume)
#   define MOCHI_ASSUME(expression) [[assume(expression)]]
#else
#   define MOCHI_ASSUME(expression)
#endif

#if __has_cpp_attribute(no_unique_address)
#   if defined(_MSVC_LANG)
        // Mention in https://en.cppreference.com/w/cpp/language/attributes/no_unique_address:
        // [[no_unique_address]] is ignored by MSVC even in C++20 mode.
        // Instead, [[msvc::no_unique_address]] is provided.
#       define MOCHI_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#   else
#       define MOCHI_NO_UNIQUE_ADDRESS [[no_unique_address]]
#   endif
#else
#   define MOCHI_NO_UNIQUE_ADDRESS
#endif

#endif // defined(__cplusplus)



#endif // !defined(__MC_MACROS_HPP_HEADER_GUARD)