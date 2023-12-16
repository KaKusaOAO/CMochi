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

	template <typename TLeft, typename T, typename... TRest>
	class Product;
	
	template <int Index, typename T>
	struct ProductValueLookup;

	template <typename TLeft, typename T, typename... TRest>
	struct ProductValueLookup<0, Product<TLeft, T, TRest...>> {
		static T Get(Product<TLeft, T, TRest...>& data) {
			return data._value;
		}
	};

	template <int Index, typename TLeft, typename T, typename... TRest> requires IsGreaterThan<Index, 0>
	struct ProductValueLookup<Index, Product<TLeft, T, TRest...>> {
		static auto Get(Product<TLeft, T, TRest...>& data) {
			static_assert(Index > 0, "Index overflowed");
			return ProductValueLookup<Index - 1, Product<TLeft, TRest...>>::Get(data._rest);
		}
	};

	template <typename TLeft, typename T, typename... TRest>
	class Product {
	public:
		template <size_t Index>
		auto Get() {
			return ProductValueLookup<Index, Product<TLeft, T, TRest...>>::Get(*this);
		}

	private:
		IApp<TLeft, T> _value;
		Product<TLeft, TRest...> _rest;
	};

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