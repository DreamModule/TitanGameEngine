#pragma once

#include “../Platform/Window.hpp”
#include “../Scene/SceneManager.hpp”
#include “../Assets/ResourceManager.hpp”
#include <memory>
#include <functional>

namespace Titan {

struct EngineConfig {
uint32_t windowWidth = 1280;
uint32_t windowHeight = 720;
std::string windowTitle = “Titan Engine”;
bool fullscreen = false;
bool vsync = true;
uint32_t maxFPS = 0;

```
float fixedTimestep = 1.0f / 60.0f;
uint32_t maxFixedStepsPerFrame = 5;
```

};

struct EngineStats {
float fps = 0.0f;
float frameTime = 0.0f;
float deltaTime = 0.0f;
uint64_t frameCount = 0;
uint32_t drawCalls = 0;
uint32_t entitiesRendered = 0;
};

class Engine {
public:
static Engine& Get() {
static Engine instance;
return instance;
}

```
bool Init(const EngineConfig& config) {
    if (initialized) return false;
    
    this->config = config;
    
    Platform::Window::CreateInfo windowInfo;
    windowInfo.width = config.windowWidth;
    windowInfo.height = config.windowHeight;
    windowInfo.title = config.windowTitle;
    windowInfo.fullscreen = config.fullscreen;
    windowInfo.vsync = config.vsync;
    
    if (!Platform::Window::Create(windowInfo)) {
        return false;
    }
    
    Platform::Window::SetEventCallback([this](const Platform::Window::Event& e) {
        OnWindowEvent(e);
    });
    
    Graphics::Device::Init(
        (HWND)Platform::Window::GetNativeWindowHandle(),
        (HDC)Platform::Window::GetNativeHandle()
    );
    
    Platform::Input::Init();
    
    initialized = true;
    running = false;
    
    return true;
}

void Run() {
    if (!initialized || running) return;
    
    running = true;
    lastFrameTime = std::chrono::high_resolution_clock::now();
    accumulator = 0.0f;
    
    while (running) {
        auto frameStart = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<float> elapsed = frameStart - lastFrameTime;
        float deltaTime = elapsed.count();
        lastFrameTime = frameStart;
        
        if (deltaTime > 0.25f) {
            deltaTime = 0.25f;
        }
        
        UpdateStats(deltaTime);
        
        Platform::Window::PollEvents();
        Platform::Input::Update();
        
        if (Platform::Window::ShouldClose()) {
            running = false;
            break;
        }
        
        accumulator += deltaTime;
        uint32_t fixedSteps = 0;
        
        while (accumulator >= config.fixedTimestep && 
               fixedSteps < config.maxFixedStepsPerFrame) {
            
            Scene::SceneManager::Get().FixedUpdateActive(config.fixedTimestep);
            
            accumulator -= config.fixedTimestep;
            fixedSteps++;
        }
        
        Scene::SceneManager::Get().UpdateActive(deltaTime);
        
        Graphics::Device::BeginFrame();
        Scene::SceneManager::Get().RenderActive();
        Graphics::Device::EndFrame();
        
        Platform::Window::SwapBuffers();
        
        Input::Manager::EndFrame();
        
        LimitFramerate(frameStart);
    }
}

void Shutdown() {
    if (!initialized) return;
    
    running = false;
    
    Assets::ResourceManager::Get().Clear();
    
    Platform::Input::Shutdown();
    Graphics::Device::Shutdown();
    Platform::Window::Destroy();
    
    initialized = false;
}

void RequestQuit() {
    running = false;
}

bool IsRunning() const {
    return running;
}

const EngineConfig& GetConfig() const {
    return config;
}

const EngineStats& GetStats() const {
    return stats;
}

Scene::Scene* CreateScene(const std::string& name) {
    return Scene::SceneManager::Get().CreateScene(name);
}

bool ActivateScene(const std::string& name) {
    return Scene::SceneManager::Get().ActivateScene(name);
}

Scene::Scene* GetActiveScene() {
    return Scene::SceneManager::Get().GetActiveScene();
}
```

private:
Engine() = default;
~Engine() = default;

```
Engine(const Engine&) = delete;
Engine& operator=(const Engine&) = delete;

void OnWindowEvent(const Platform::Window::Event& e) {
    if (e.type == Platform::Window::EventType::WindowClose) {
        running = false;
    }
    else if (e.type == Platform::Window::EventType::KeyPress) {
        Platform::Input::OnKeyEvent(e.key.keycode, true, e.key.repeat);
    }
    else if (e.type == Platform::Window::EventType::KeyRelease) {
        Platform::Input::OnKeyEvent(e.key.keycode, false, false);
    }
    else if (e.type == Platform::Window::EventType::MouseButtonPress) {
        Platform::Input::OnMouseButtonEvent(e.mouseButton.button, true, 
                                            e.mouseButton.x, e.mouseButton.y);
    }
    else if (e.type == Platform::Window::EventType::MouseButtonRelease) {
        Platform::Input::OnMouseButtonEvent(e.mouseButton.button, false,
                                            e.mouseButton.x, e.mouseButton.y);
    }
    else if (e.type == Platform::Window::EventType::MouseMove) {
        Platform::Input::OnMouseMoveEvent(e.mouseMove.x, e.mouseMove.y);
    }
    else if (e.type == Platform::Window::EventType::MouseScroll) {
        Platform::Input::OnMouseScrollEvent(e.mouseScroll.xOffset, e.mouseScroll.yOffset);
    }
}

void UpdateStats(float deltaTime) {
    stats.deltaTime = deltaTime;
    stats.frameTime = deltaTime * 1000.0f;
    stats.frameCount++;
    
    if (deltaTime > 0.0f) {
        stats.fps = 1.0f / deltaTime;
    }
}

void LimitFramerate(const std::chrono::high_resolution_clock::time_point& frameStart) {
    if (config.maxFPS == 0) return;
    
    auto targetFrameTime = std::chrono::microseconds(1000000 / config.maxFPS);
    auto frameEnd = std::chrono::high_resolution_clock::now();
    auto frameDuration = frameEnd - frameStart;
    
    if (frameDuration < targetFrameTime) {
        std::this_thread::sleep_for(targetFrameTime - frameDuration);
    }
}

EngineConfig config;
EngineStats stats;

bool initialized = false;
bool running = false;

std::chrono::high_resolution_clock::time_point lastFrameTime;
float accumulator = 0.0f;
```

};

}
