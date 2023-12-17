/// meta.hpp
/// --
/// Metaprogramming-related header file.

#pragma once

#if defined(__cplusplus)
#ifndef __MC_META_HPP_HEADER_GUARD
#define __MC_META_HPP_HEADER_GUARD

#include <Mochi/core.hpp>

namespace __MC_NAMESPACE {
    template <typename T, typename TBase>
    struct IsDerived {
        static const Bool Value = std::is_base_of<TBase, T>::value;
    };

    template <typename TBase, typename... T>
    struct IsBasedOn {
        static const Bool Value = (std::is_base_of<TBase, T>::value && ...);
    };

    template <typename T>
    struct HasInvokeOperator {
        template <typename TRet, typename... TArgs>
        class WithSignature {
            using Yes = char[1];
            using No = char[2];

            struct Fallback { TRet operator()(TArgs...); };
            struct Derived : T, Fallback {};

            template <typename TType, TType>
            struct Check;

            template <typename TUnused>
            static Yes& Test(...);

            template <typename TCheck>
            static No& Test(Check<TRet(Fallback::*)(TArgs...), &TCheck::operator()>* ptr);

        public:
            static const bool Value = sizeof(Test<Derived>(nullptr)) == sizeof(Yes);
        };
    };

    template <typename T, typename TBase>
    constexpr Bool IsDerivedV = IsDerived<T, TBase>::Value;

    template <typename TBase, typename... T>
    constexpr Bool IsBasedOnV = IsBasedOn<TBase, T...>::Value;

    template <typename T, typename TRet, typename... TArgs>
    constexpr Bool HasInvokeOperatorV = HasInvokeOperator<T>::template WithSignature<TRet, TArgs...>::Value;

    // "Concepts" are introduced since C++20
    #if defined(MOCHI_CPLUSPLUS_HAS_CXX20)
    namespace Concepts {

        template <typename T, typename TBase>
        concept IsDerived = IsDerivedV<T, TBase>;

        template <Bool B>
        concept IsTrue = B;

    };
    #endif // defined(MOCHI_CPLUSPLUS_HAS_CXX20)

    template <int Index, typename... TTypes>
    #if defined(MOCHI_CPLUSPLUS_HAS_CXX20)
        requires Concepts::IsTrue<(Index >= 0)>
    #endif // defined(MOCHI_CPLUSPLUS_HAS_CXX20)
    struct NthTypeContext {};
    
    template <typename T, typename... TRest>
    struct NthTypeContext<0, T, TRest...> {
        using Type = T;
    };
    
    template <int Index, typename T, typename... TRest>
    #if defined(MOCHI_CPLUSPLUS_HAS_CXX20)
        requires Concepts::IsTrue<(Index > 0)>
    #endif // defined(MOCHI_CPLUSPLUS_HAS_CXX20)
    struct NthTypeContext<Index, T, TRest...> {
        using Type = NthTypeContext<Index - 1, TRest...>::Type;
    };

    /// @brief Helper type of `NthTypeContext` to get the Nth type directly.
    /// @tparam ...TTypes A pack parameter of types.
    /// @tparam Index The 0-based index of the desired type.
    template <int Index, typename... TTypes>
    #if defined(MOCHI_CPLUSPLUS_HAS_CXX20)
        requires Concepts::IsTrue<(Index >= 0)>
    #endif // defined(MOCHI_CPLUSPLUS_HAS_CXX20)
    using NthTypeOf = NthTypeContext<Index, TTypes...>::Type;

    template <typename T, typename TOther>
    struct IsSameTypeContext {
        static const Bool Value = False;
    };

    template <typename T>
    struct IsSameTypeContext<T, T> {
        static const Bool Value = True;
    };

    template <typename T, typename TOther>
    constexpr Bool IsSameType = IsSameTypeContext<T, TOther>::Value;

    template <typename... TTypes>
    struct TypeStack;

    template <int Size, typename... TRest>
    struct SplitContext {};

    template < typename... TRest>
    struct SplitContext<0, TRest...> {
        using LeftStack = TypeStack<>;
        using RightStack = TypeStack<TRest...>;
    };

    template <typename T, typename... TRest>
    struct SplitContext<1, T, TRest...> {
        using LeftStack = TypeStack<T>;
        using RightStack = TypeStack<TRest...>;
    };

    template <int Size, typename T, typename... TRest>
    struct SplitContext<Size, T, TRest...> {
        using LeftStack = TypeStack<T>::Concat<SplitContext<Size - 1, TRest...>::LeftStack>;
        using RightStack = SplitContext<Size - 1, TRest...>::RightStack;
    };

    template <typename... TTypes>
    struct TypeStack {
        template <int Index>
        using NthOfType = NthTypeOf<Index, TTypes...>;
        
        using FirstType = NthOfType<0>;
        static const auto TypeCount = sizeof...(TTypes);

        template <typename... TTail>
        struct AppendContext {
            using Type = TypeStack<TTypes..., TTail...>;
            using Split = SplitContext<sizeof...(TTypes), TTypes..., TTail...>;
        };

        template <typename... TTail>
        using Append = AppendContext<TTail...>::Type;

        template <typename TSource>
        struct ConcatContext {};

        template <typename... TTail>
        struct ConcatContext<TypeStack<TTail...>> {
            using Type = TypeStack<TTypes..., TTail...>;
        };

        template <typename TSource>
        using Concat = ConcatContext<TSource>::Type;

        template <template <typename...> typename TTarget>
        using ApplyTo = TTarget<TTypes...>;

        template <int Size>
#if defined(MOCHI_CPLUSPLUS_HAS_CXX20)
            requires Concepts::IsTrue<(Size >= 0 && Size <= sizeof...(TTypes))>
#endif // defined(MOCHI_CPLUSPLUS_HAS_CXX20)
        struct SplitAt {
            using LeftStack  = SplitContext<Size, TTypes...>::LeftStack;
            using RightStack = SplitContext<Size, TTypes...>::RightStack;
        };
    };

};

#endif
#endif