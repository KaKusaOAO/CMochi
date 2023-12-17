#pragma once

#if defined(__cplusplus)
#ifndef __MC_DATA_HPP_HEADER_GUARD
#define __MC_DATA_HPP_HEADER_GUARD

#include <Mochi/foundation.hpp>

namespace __MC_NAMESPACE {
    class IK1 {};

    class IAppU {};

    template <typename T> requires IsDerived<T, IK1>
    class IAppLeft : public IAppU {};

    template <typename T>
    class IAppRight : public IAppU {};

    template <typename TLeft, typename TRight> requires IsDerived<TLeft, IK1>
    class IApp : public IAppLeft<TLeft>, public IAppRight<TRight> {};

    // template <typename TLeft, typename... TRest>
    // class Product {
    // public:
    //     int GetCount() { return 0; }
    // };
    
    // template <int Index, typename T>
    // struct ProductValueLookup;

    // template <typename TLeft, typename T, typename... TRest>
    // struct ProductValueLookup<0, Product<TLeft, T, TRest...>> {
    //     constexpr static Bool CanLookup = true;
    // 	static IApp<TLeft, T> Get(Product<TLeft, T, TRest...>& data) {
    // 		return data.Value;
    // 	}
    // };
    
    // template <typename TLeft, typename... TRest>
    // struct ProductValueLookup<0, Product<TLeft, TRest...>> {
    //     constexpr static Bool CanLookup = false;
    // };

    // template <int Index, typename TLeft, typename T, typename... TRest>
    // requires IsTrue<ProductValueLookup<Index - 1, Product<TLeft, TRest...>>::CanLookup>
    // struct ProductValueLookup<Index, Product<TLeft, T, TRest...>> {
    //     constexpr static Bool CanLookup = true;
    // 	static auto Get(Product<TLeft, T, TRest...>& data) {
    // 		return ProductValueLookup<Index - 1, Product<TLeft, TRest...>>::Get(data.Rest);
    // 	}
    // };
    
    // template <typename TLeft, typename T, typename... TRest>
    // class Product<TLeft, T, TRest...> {
    // public:
    //     Product(const T& val, const TRest&... rest)
    //     : Rest(rest...) {}
        
    // 	template <size_t Index>
    // 	auto Get() {
    // 		return ProductValueLookup<Index, Product<TLeft, T, TRest...>>::Get(*this);
    // 	}
        
    //     int GetCount() { return sizeof...(TRest) + 1; }

    //     IApp<TLeft, T> Value;
    //     Product<TLeft, TRest...> Rest;
    // };

    template <typename TLeft, typename T, typename... TRest>
    using Product = DataStructure<IApp<TLeft, T>, IApp<TLeft, TRest>...>;

    class IKind1U {
    public:
        class IMu : public IK1 {};
    };

    template <typename T> requires IsDerived<T, IK1>
    class IKind1Left : public IKind1U, public IAppRight<T> {};
    
    template <typename T> requires IsDerived<T, IKind1U::IMu>
    class IKind1Right : public IKind1U, public IAppLeft<T> {};

    template <typename TLeft, typename TRight> requires IsDerived<TLeft, IK1> && IsDerived<TRight, IKind1U::IMu>
    class IKind1 : public IKind1Left<TLeft>, public IKind1Right<TRight>, public IApp<TRight, TLeft> {};
};

#endif 
#endif
