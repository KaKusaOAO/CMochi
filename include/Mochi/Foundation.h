#pragma once

#if defined(__cplusplus)
#ifndef __MOCHI_FOUNDATION_H_HEADER_GUARD
#define __MOCHI_FOUNDATION_H_HEADER_GUARD

#include <Mochi/Core.h>
#include <Mochi/Meta.h>
#include <array>
#include <iostream>
#include <numbers>
#include <list>
#include <type_traits>
#include <future>
#include <execution>
#include <queue>
#include <sstream>
#include <functional>

// We are using parseInt("Foundation", 31).toString(32)
#define __MC_INTERNAL __Intrnl_bs2ot97vij__

namespace MOCHI_NAMESPACE {

    template <class TSrc, class TDst>
    Bool TryCastRef(const Handle<TSrc> obj, Handle<TDst>& out) {
        auto result = std::dynamic_pointer_cast<TDst>(obj);
        if (!result) {
            return false;
        }

        out = result;
        return true;
    }

    template <class TDst, class TSrc>
    Handle<TDst> TryCastRef(const Handle<TSrc> obj) {
        return std::dynamic_pointer_cast<TDst>(obj);
    }

    template <class TDst, class TSrc>
    Handle<TDst> CastRef(const Handle<TSrc> obj) {
        Handle<TDst> result;
        if (!::MOCHI_NAMESPACE::TryCastRef(obj, result)) {
            std::stringstream str;
            str << "Cannot cast object " << obj << " to type " << typeid(TDst).name();
            throw std::runtime_error(str.str());
        }

        return result;
    }
    
    template <typename T, typename... TArgs>
    Handle<T> CreateRef(TArgs... args) {
        return std::make_shared<T>(args...);
    }

    template <typename TBase, typename T>
    #if defined(MOCHI_CPLUSPLUS_HAS_CXX20)
        requires Concepts::IsDerived<T, std::enable_shared_from_this<TBase>>
    #endif // defined(MOCHI_CPLUSPLUS_HAS_CXX20)
    Handle<T> GetRef(T* obj) {
        return ::MOCHI_NAMESPACE::CastRef<T>(obj->shared_from_this());
    }

    template <class T, class TBase> 
    #if defined(MOCHI_CPLUSPLUS_HAS_CXX20)
        requires Concepts::IsDerived<T, TBase>
    #endif // defined(MOCHI_CPLUSPLUS_HAS_CXX20)
    Handle<T> AssertSubType(Handle<TBase> value) {
        std::stringstream str;
        
        if (!value) {
            str << "Expected object of type " << typeid(T).name() << " but received null instead.";
            throw std::runtime_error(str.str());
        }
        
        if (Handle<T> result = TryCastRef<T>(value)) {
            return result;
        }
        
        str << "Object " << value << " (typeof " << typeid(TBase).name() << ")";
        str << " must be derived type " << typeid(T).name() << " to be used in this context.";
        throw std::runtime_error(str.str());
    }

    class IDisposable {
    public:
        virtual void Dispose();
    };

    template <typename T>
    class Lazy {
    public:
        using Initiator = std::function<T()>;

        Lazy(Initiator initiator) : _initiator(initiator), _initialized(false), _value{} {}

        T& GetValue() {
            if (!_initialized) {
                _value = _initiator();
                _initialized = true;
            }

            return _value;
        }

        operator T() {
            return GetValue();
        }

        T& operator*() {
            return GetValue();
        }

        Lazy<T>& operator=(const Lazy<T>& other) {
            _initiator = other._initiator;
            _initialized = false;
            return *this;
        }

    private:
        Initiator _initiator;
        Bool _initialized;
        T _value;
    };

    class PreconditionFailedException : public std::exception {
    private:
        std::string _message;
    public:
        PreconditionFailedException(std::string message);
        const char * what() const noexcept override;
    };

    class Preconditions {
    private:
        static void InternalEnsure(Bool test, void* val, std::string message);
    public:
        template <class N>
        static void IsPositive(N value, std::string name) {
            InternalEnsure(value >= 0, &value, name + " cannot be less than 0.");
        }
    };

    namespace Math {
        constexpr static double DegToRad = std::numbers::pi / 180.0;
        constexpr static double RadToDeg = 180.0 / std::numbers::pi;
    }

    struct Color {
        UInt8  R;
        UInt8  G;
        UInt8  B;
        UInt32 GetRGB() const;

        Color(UInt32 hex);
        Color(double r, double g, double b);

        void Normalize(double *outR, double *outG, double *outB);
        void ToHsv(double *outHue, double *outSaturation, double *outValue);

        static Color FromHsv(double hue, double saturation, double value);
    };

    class Enumerables {
    public:
        template<typename Out, typename In, template <typename> typename E>
        static std::vector<Out> Select(E<In> &input, std::function<Out(In)> convert) {
            std::vector<Out> result;
            for (In item : input) {
                result.push_back(convert(item));
            }
            
            return result;
        }
    };

    class Future {
    public:
        template<typename C>
        static std::future<void> WhenAll(C &futures) {
            std::promise<void> promise;
            
            for (auto future : futures) {
                future.wait();
            }
            
            promise.set_value();
            return promise.get_future();
        }
    };

    template <class T>
    class AsyncEventHandler {
    public:
        using HandlerEntry = std::shared_ptr<T>;

    private:
        std::list<HandlerEntry> _handlers;

    public:
        AsyncEventHandler() : _handlers() {
            
        }
        
        std::list<HandlerEntry> GetHandlers() {
            return _handlers;
        }
        
        void AddHandler(HandlerEntry handler) {
            _handlers.push_back(handler);
        }
        
        void RemoveHandler(HandlerEntry handler) {
            _handlers.remove(handler);
        }
        
        std::future<void> InvokeAsync(std::function<std::future<void>(T)> invoke) {
            std::vector<std::future<void>> tasks = Enumerables::Select(_handlers, invoke);
            return Future::WhenAll(tasks);
            
            //        std::promise<void> promise;
            //
            //        std::thread worker([&]() {
            //            std::vector<std::thread> subthreads;
            //
            //            for (T handler : _handlers) {
            //                std::thread subthread([&]() {
            //                    invoke(handler).wait();
            //                });
            //                subthreads.push_back(subthread);
            //            }
            //
            //            for (std::thread &t : subthreads) {
            //                t.join();
            //            }
            //
            //            promise.set_value();
            //        });
            //
            //        return promise.get_future();
        }
    };
    
    template <typename TRet, typename... TArgs>
    class FuncDelegate {
    public:
        using Type = std::function<TRet (TArgs...)>;
        FuncDelegate(Type func) : _func(func) {}
        
        inline TRet operator()(TArgs... args) {
            return _func(args...);
        }
        
        inline Type GetFunction() {
            return _func;
        }
        
    private:
        Type _func;
    };
    
    class IDataStructure {
    public:
        virtual int GetCount() = 0;
    };
    
    template <typename... T>
    class DataStructure : public IDataStructure {
    public:
        int GetCount() override { return 0; }
    };
    
    template <typename T, typename... TRest>
    class DataStructure<T, TRest...> : public IDataStructure {
    public:
        DataStructure() : _value(), _rest() {}
        DataStructure(T val, TRest... rest) : _value(val), _rest(rest...) {}

    private:
        T _value;
        DataStructure<TRest...> _rest;
    
        template <int Index>
        class Visitor {
        public:
            using Type = NthTypeOf<0, TRest...>;
            Visitor(DataStructure<T, TRest...>& data) : _data(data) {}
            
            Type Visit() {
                return _data._rest.template CreateVisitor<Index - 1>().Visit();
            }
            
            void Set(Type val) {
                _data._rest.template CreateVisitor<Index - 1>().Set(val);
            }

        private:
            DataStructure<T, TRest...>& _data;
        };

        template <>
        class Visitor<0> {
        public:
            using Type = T;
            Visitor(DataStructure<T, TRest...>& data) : _data(data) {}
            
            Type Visit() {
                return _data._value;
            }
            
            void Set(Type val) {
                _data._value = val;
            }

        private:
            DataStructure<T, TRest...>& _data;
        };

    public:
        template <int Index>
        Visitor<Index> CreateVisitor() {
            return Visitor<Index>(*this);
        }

        template <int Index>
        NthTypeOf<Index, T, TRest...>::Type Get() {
            return CreateVisitor<Index>().Visit();
        }
        
        template <int Index>
        void Set(NthTypeOf<Index, T, TRest...>::Type val) {
            CreateVisitor<Index>().Set(val);
        }
        
        int GetCount() override {
            return sizeof...(TRest) + 1;
        }
    };
    
    namespace __MC_INTERNAL {
        template <typename... THead>
        struct CurryVariadicContext {
            // This struct is not meant to be created.
            CurryVariadicContext() = delete;
            
            template <typename... TTail>
            struct Append {
                // This struct is not meant to be created.
                Append() = delete;
                
                template <typename TRet>
                class Result {
                public:
                    using FuncType = std::function<TRet(THead..., TTail...)>;
                    using PureFuncType = FuncDelegate<FuncDelegate<TRet, TTail...>, THead...>;
                    Result(FuncType func) : _func(func) {}
                    
                private:
                    class Continuation {
                    public:
                        using FuncType = std::function<TRet(TTail...)>;
                        
                        Continuation(FuncType func) : _func(func) {}
                        
                        FuncDelegate<TRet, TTail...> ToFunc() {
                            return FuncDelegate<TRet, TTail...>([this](TTail... args) {
                                return Invoke(args...);
                            });
                        }
                        
                        TRet operator()(TTail... args) { return Invoke(args...); }
                        TRet Invoke(TTail... args) {
                            return _func(args...);
                        }
                        
                    private:
                        FuncType _func;
                    };
                    
                    FuncType _func;
                    
                public:
                    // FuncDelegate<Continuation, THead...> ToFunc() {
                    //     auto func = _func;
                    //     return FuncDelegate<Continuation, THead...>([func](THead... args) {
                    //         return Continuation([func, args...](TTail... rest) {
                    //             return func(args..., rest...);
                    //         });
                    //     });
                    // }
                    
                    FuncDelegate<FuncDelegate<TRet, TTail...>, THead...> ToPureFunc() {
                        auto func = _func;
                        return FuncDelegate<FuncDelegate<TRet, TTail...>, THead...>([func](THead... args) {
                            return FuncDelegate<TRet, TTail...>([func, args...](TTail... rest) {
                                return func(args..., rest...);
                            });
                        });
                    }
                    
                    // Continuation operator()(THead... args) { return Invoke(args...); }
                    // Continuation Invoke(THead... args) {
                    //     auto func = _func;
                    //     return Continuation([func, args...](TTail... rest) {
                    //         return func(args..., rest...);
                    //     });
                    // }
                };
                
                template <typename TRet>
                using ResultFunc = std::function<
                    std::function<TRet(TTail...)>
                    (THead...)
                >;
            };
            
            template <typename TRet>
            using FuncType = std::function<TRet(THead...)>;
        };
        
        template <typename TRet, typename TSplitContext>
        struct CurryTypeSplit {
            static const Bool IsValid = False;
            using LeftContext = void;
            using RightContext = void;
            using Result = void;
        };

        template <int Size, typename TRet, typename... THead>
        struct CurryTypeSplit<TRet, SplitContext<Size, THead...>> {
            static const Bool IsValid = True;
            using LeftContext  = SplitContext<Size, THead...>::LeftStack::template ApplyTo<CurryVariadicContext>;
            using RightContext = SplitContext<Size, THead...>::RightStack::template ApplyTo<LeftContext::template Append>;
            using Result = RightContext::template Result<TRet>;
        };
    };
    
    template <int Size, typename TRet, typename... TArgs>
    using CurriedFunction = typename __MC_INTERNAL::CurryTypeSplit<TRet, SplitContext<Size, TArgs...>>::Result::PureFuncType;
    
    template <int Size, typename TRet, typename... TArgs>
    CurriedFunction<Size, TRet, TArgs...> MakeCurry(std::function<TRet(TArgs...)> func) {
        static_assert(Size > 0,                "Size must be greater than 0.");
        static_assert(Size < sizeof...(TArgs), "Size must not reach total argument count.");
        
        using ResultType = typename __MC_INTERNAL::CurryTypeSplit<TRet, SplitContext<Size, TArgs...>>::Result;
        
        auto result = ResultType(func);
        return result.ToPureFunc();
    }
    
    template <int Size, typename TRet, typename... TArgs>
    CurriedFunction<Size, TRet, TArgs...> MakeCurry(FuncDelegate<TRet, TArgs...> func) {
        return MakeCurry<Size, TRet, TArgs...>(func.GetFunction());
    }
}

#undef __MC_INTERNAL

#endif
#endif
