#include "Engine.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cassert>

using namespace Engine;

struct QuickTestSystem : public SystemManager::ISystem {
    int updates = 0;
    int fixeds = 0;
    int renders = 0;
    int stopAfterUpdates = 120;
    void OnInit() override {
        std::cout << "[QuickTestSystem] OnInit\n";
    }
    void Update(float dt) override {
        ++updates;
        if ((updates % 30) == 0) std::cout << "[QuickTestSystem] Update " << updates << " dt=" << dt << "\n";
        if (updates >= stopAfterUpdates) EngineCore::RequestQuit();
    }
    void FixedUpdate(float dt) override {
        ++fixeds;
        if ((fixeds % 60) == 0) std::cout << "[QuickTestSystem] FixedUpdate " << fixeds << " fd=" << dt << "\n";
    }
    void Render() override {
        ++renders;
        if ((renders % 60) == 0) std::cout << "[QuickTestSystem] Render " << renders << "\n";
    }
    void Shutdown() override {
        std::cout << "[QuickTestSystem] Shutdown\n";
    }
};

struct DirectSystem : public SystemManager::ISystem {
    void OnInit() override { std::cout << "[DirectSystem] OnInit\n"; }
    void Update(float dt) override { std::cout << "[DirectSystem] Update dt=" << dt << "\n"; }
    void FixedUpdate(float dt) override { std::cout << "[DirectSystem] FixedUpdate dt=" << dt << "\n"; }
    void Render() override { std::cout << "[DirectSystem] Render\n"; }
    void Shutdown() override { std::cout << "[DirectSystem] Shutdown\n"; }
};

int main() {
    EngineConfig cfg;
    cfg.width = 640;
    cfg.height = 480;
    cfg.title = "TitanEngine Full Test";
    cfg.fixedStep = 1.0f / 60.0f;
    cfg.maxFixedStepsPerFrame = 4;
    cfg.targetFps = 60;
    cfg.workerThreads = 2;
    cfg.autoShutdown = false;

    std::cout << "[Test] Calling Time::Init and Input::Manager::Init before engine init\n";
    Time::Init();
    Input::Manager::Init();

    std::cout << "[Test] EngineCore::Init\n";
    EngineCore::Init(cfg);

    std::cout << "[Test] Query config and steps\n";
    const EngineConfig& rcfg = EngineCore::GetConfig();
    assert(rcfg.width == cfg.width);
    std::cout << "[Test] FixedStep = " << EngineCore::GetFixedStep() << "\n";

    std::cout << "[Test] Registering callbacks\n";
    EngineCore::RegisterFixedStepCallback([](float dt){
        static int c = 0;
        if (++c % 120 == 0) std::cout << "[CB] Fixed callback count=" << c << " dt=" << dt << "\n";
    });
    EngineCore::RegisterUpdateCallback([](float dt){
        static int u = 0;
        if (++u % 120 == 0) std::cout << "[CB] Update callback count=" << u << " dt=" << dt << "\n";
    });
    EngineCore::RegisterShutdownCallback([](){
        std::cout << "[CB] Shutdown callback fired\n";
    });

    std::cout << "[Test] Direct calls to subsystem APIs\n";
    SystemManager::Init();
    SystemManager::RegisterSystem(std::make_unique<DirectSystem>(), 10);
    SystemManager::UpdateAll(1.0f/60.0f);
    SystemManager::FixedUpdateAll(1.0f/60.0f);
    SystemManager::RenderAll();

    Graphics::Renderer::BeginFrame();
    Graphics::Renderer::EndFrame();
    Graphics::Renderer::Present();

    Scene::SceneManager::Init();
    Scene::SceneManager::LoadAsync("test_scene");
    Scene::SceneManager::Activate("test_scene");
    Scene::SceneManager::UpdateActive(1.0f/60.0f);
    Scene::SceneManager::RenderActive();

    Physics::PhysicsSystem::Step(1.0f/60.0f);

    std::cout << "[Test] Enqueue background task\n";
    EngineCore::EnqueueBackgroundTask([](){
        std::cout << "[BG] background work start\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "[BG] background work done\n";
    });

    std::cout << "[Test] Enqueue main-thread task\n";
    EngineCore::EnqueueMainThreadTask([](){
        std::cout << "[MainTask] main thread task executed\n";
    });

    std::cout << "[Test] Registering systems through EngineCore\n";
    EngineCore::RegisterSystem(std::make_unique<QuickTestSystem>(), 0);

    std::cout << "[Test] Also register a system directly with SystemManager\n";
    SystemManager::RegisterSystem(std::make_unique<DirectSystem>(), 5);

    std::cout << "[Test] Start engine in separate thread\n";
    std::thread engineThread([](){ EngineCore::Run(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::cout << "[Test] While engine running: enqueue several main and bg tasks\n";
    for (int i = 0; i < 5; ++i) {
        EngineCore::EnqueueBackgroundTask([i]() {
            std::cout << "[BG] task " << i << " running\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            std::cout << "[BG] task " << i << " done\n";
        });
        EngineCore::EnqueueMainThreadTask([i]() {
            std::cout << "[Main] task " << i << " executed on main thread\n";
        });
    }

    std::cout << "[Test] Wait for background tasks to finish\n";
    EngineCore::WaitForBackgroundTasks();
    std::cout << "[Test] Background tasks completed\n";

    std::cout << "[Test] Sleep briefly to let engine loop progress\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "[Test] RequestQuit from test harness\n";
    EngineCore::RequestQuit();

    if (engineThread.joinable()) engineThread.join();
    std::cout << "[Test] Engine thread joined\n";

    std::cout << "[Test] Call subsystems shutdown paths explicitly\n";
    SystemManager::Shutdown();
    Scene::SceneManager::Shutdown();
    Graphics::Renderer::Shutdown();
    Platform::Window::Destroy();

    std::cout << "[Test] Final EngineCore::Shutdown\n";
    EngineCore::Shutdown();

    std::cout << "[Test] Verifying ShouldQuit and last delta\n";
    std::cout << "[Test] ShouldQuit()=" << (EngineCore::ShouldQuit() ? "true" : "false") << "\n";
    std::cout << "[Test] LastDelta=" << EngineCore::GetLastDelta() << "\n";

    std::cout << "[Test] All tests finished\n";
    return 0;
}
