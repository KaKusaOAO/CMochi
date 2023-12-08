//
//  platform.hpp
//  MiaCrate++
//
//  Created by 咔咔 on 2023/12/6.
//

#if defined(__cplusplus)
#ifndef __MC_PLATFORM_HPP_HEADER_GUARD
#define __MC_PLATFORM_HPP_HEADER_GUARD

#include <SDL3/SDL.h>
#include <Mochi/common.hpp>
#include <Mochi/core.hpp>
#include <thread>
#include <exception>

namespace Mochi {

class RenderSystem {
private:
    static const Int32 MinimumAtlasTextureSize = 1024;
    
    static Bool _isInInit;
    static std::thread::id _renderThreadId;
    static Int32 _maxSupportedTextureSize;
    
    static std::exception ConstructThreadException() {
        return std::runtime_error("RenderSystem called from wrong thread.");
    }
    
public:
    static Bool IsOnRenderThread() {
        auto id = std::this_thread::get_id();
        return _renderThreadId == id;
    }
    
    static Bool IsOnRenderThreadOrInit() {
        return _isInInit || IsOnRenderThread();
    }
    
    static Bool IsOnGameThread() {
        return true;
    }
    
    static Bool IsInInitPhase() {
        return true;
    }
    
    static void AssertOnRenderThreadOrInit() {
        if (!_isInInit && IsOnRenderThread()) {
            throw ConstructThreadException();
        }
    }
    
    static void AssertOnRenderThread() {
        if (!IsOnRenderThread()) {
            throw ConstructThreadException();
        }
    }
    
    static void AssertOnGameThreadOrInit() {
        if (!_isInInit && IsOnGameThread()) {
            throw ConstructThreadException();
        }
    }
    
    static void AssertOnGameThread() {
        if (!IsOnGameThread()) {
            throw ConstructThreadException();
        }
    }
    
    static Int32 GetMaxSupportedTextureSize() {
        if (_maxSupportedTextureSize != -1) return _maxSupportedTextureSize;
        
        AssertOnRenderThreadOrInit();
        
        // TODO: Properly get the max supported texture size
        
        Int32 i = 32768;
        _maxSupportedTextureSize = std::max(i, MinimumAtlasTextureSize);
        return _maxSupportedTextureSize;
    }
    
    static void AssertInInitPhase() {
        
    }
};

Int32 RenderSystem::_maxSupportedTextureSize = -1;

class Monitor {
    
};

// typedef void (* MonitorCreator)();
typedef std::function<void()> MonitorCreator;

class ScreenManager : public IDisposable {
private:
    MonitorCreator _monitorCreator = nullptr;
    
public:
    ScreenManager(MonitorCreator creator) {
        RenderSystem::AssertInInitPhase();
        _monitorCreator = creator;
    }
    
    
};

}

#endif /* platform_hpp */
#endif
