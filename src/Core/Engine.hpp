#pragma once
#include <atomic>
#include <functional>
#include <memory>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <unordered_map>
#include <algorithm>
#include <cstdint>

namespace Platform {
namespace Window {
bool ShouldClose();
void PollEvents();
void SwapBuffers();
void Create(int width, int height, const char* title, void* nativeHandle = nullptr);
void Destroy();
void* GetNativeHandle();
}
}

namespace Time {
void Init();
void Update();
float Delta();
float FixedStep();
double Accumulated();
void SetFixedStep(float s);
}

namespace Input {
namespace Manager {
void Init();
void Update();
void EndFrame();
}
}

namespace SystemManager {
struct ISystem {
    virtual ~ISystem() = default;
    virtual void OnInit() {}
    virtual void Update(float dt) {}
    virtual void FixedUpdate(float dt) {}
    virtual void Render() {}
    virtual void Shutdown() {}
};
void Init();
void RegisterSystem(std::unique_ptr<ISystem> sys, int priority = 0);
void UpdateAll(float dt);
void FixedUpdateAll(float dt);
void RenderAll();
void Shutdown();
}

namespace Graphics {
namespace Renderer {
void Init(void* device);
void BeginFrame();
void EndFrame();
void Present();
void Shutdown();
}
}

namespace Scene {
namespace SceneManager {
void Init();
void UpdateActive(float dt);
void RenderActive();
void Shutdown();
void LoadAsync(const std::string& name);
void Activate(const std::string& name);
}
}

namespace Physics {
namespace PhysicsSystem {
void Step(float dt);
}
}

namespace Engine {

struct EngineConfig {
    int width = 1280;
    int height = 720;
    const char* title = "Titan Engine";
    void* nativeWindowHandle = nullptr;
    float fixedStep = 1.0f / 60.0f;
    bool vSync = true;
    int maxFixedStepsPerFrame = 5;
    bool autoShutdown = true;
    int targetFps = 0;
    int workerThreads = 0;
};

class EngineCore {
public:
    static void Init(const EngineConfig& cfg);
    static void Run();
    static void RequestQuit();
    static void Shutdown();
    static bool ShouldQuit();
    static void RegisterFixedStepCallback(std::function<void(float)> cb);
    static void RegisterUpdateCallback(std::function<void(float)> cb);
    static void RegisterShutdownCallback(std::function<void()> cb);
    static void RegisterSystem(std::unique_ptr<SystemManager::ISystem> sys, int priority = 0);
    static void EnqueueMainThreadTask(std::function<void()> task);
    static void EnqueueBackgroundTask(std::function<void()> task);
    static void WaitForBackgroundTasks();
    static const EngineConfig& GetConfig();
    static float GetLastDelta();
    static float GetFixedStep();

private:
    EngineConfig config_;
    std::atomic<bool> requestQuit_{false};
    std::atomic<bool> running_{false};
    std::mutex cbMutex_;
    std::vector<std::function<void(float)>> fixedStepCbs_;
    std::vector<std::function<void(float)>> updateCbs_;
    std::vector<std::function<void()>> shutdownCbs_;
    std::mutex systemMutex_;
    struct SystemEntry { std::unique_ptr<SystemManager::ISystem> sys; int priority; };
    std::vector<SystemEntry> systems_;
    std::deque<std::function<void()>> mainThreadTasks_;
    std::mutex mainTasksMutex_;
    std::condition_variable mainTasksCv_;
    std::deque<std::function<void()>> bgTasks_;
    std::mutex bgTasksMutex_;
    std::condition_variable bgTasksCv_;
    std::vector<std::thread> workers_;
    std::atomic<int> activeBgTasks_{0};
    std::mutex workerLifecycleMutex_;
    std::condition_variable workerLifecycleCv_;
    float lastDelta_{0.0f};
    EngineCore() = default;
    ~EngineCore() = default;
    static EngineCore& Instance();
    void init(const EngineConfig& cfg);
    void startWorkers();
    void stopWorkers();
    void workerThreadLoop();
    void run();
    void requestQuit();
    bool shouldQuit() const;
    void shutdown();
    void performShutdown();
    void registerFixedCallback(std::function<void(float)> cb);
    void registerUpdateCallback(std::function<void(float)> cb);
    void registerShutdownCallback(std::function<void()> cb);
    void registerSystem(std::unique_ptr<SystemManager::ISystem> sys, int priority);
    void enqueueMainThreadTask(std::function<void()> task);
    void enqueueBackgroundTask(std::function<void()> task);
    void waitForBackgroundTasks();
};

}
