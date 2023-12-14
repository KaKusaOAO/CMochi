//
//  core.hpp
//  MiaCrate++
//
//  Created by 咔咔 on 2023/12/6.
//

#if defined(__cplusplus)
#ifndef __MC_CORE_HPP_HEADER_GUARD
#define __MC_CORE_HPP_HEADER_GUARD

#include <numbers>
#include <list>
#include <type_traits>
#include <future>
#include <execution>
#include <queue>
#include <memory>
#include <sstream>
#include <Mochi/common.hpp>

namespace Mochi {

template <class T, class Base>
std::shared_ptr<T> AssertSubType(std::shared_ptr<Base> value) {
    static_assert(std::is_base_of<Base, T>::value, "The given T class must derive from Base (argument).");
    
    std::stringstream str;
    
    if (!value) {
        str << "Expected object of type " << typeid(T).name() << " but received null instead.";
        throw std::runtime_error(str.str());
    }
    
    auto result = std::dynamic_pointer_cast<T>(value);
    if (!result) {
        str << "Object " << value << " (typeof " << typeid(Base).name() << ")";
        str << " must be derived type " << typeid(T).name() << " to be used in this context.";
        throw std::runtime_error(str.str());
    }
    
    return result;
}

class IDisposable {
public:
    virtual void Dispose();
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
    UInt32 RGB();
    
    Color(UInt32 hex);
    Color(double r, double g, double b);

    void Normalize(double *outR, double *outG, double *outB);
    void ToHsv(double *outHue, double *outSaturation, double *outValue);

    static Color FromHsv(double hue, double saturation, double value);
};

class Enumerables {
public:
    template<typename Out, typename In>
    static std::vector<Out> Select(std::vector<In> &input, std::function<Out(In)> convert) {
        std::vector<Out> result;
        for (In item : input) {
            result.push_back(convert(item));
        }
        
        return result;
    }
    
    template<typename Out, typename In>
    static std::vector<Out> Select(std::list<In> &input, std::function<Out(In)> convert) {
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

}

#endif /* core_hpp */
#endif
