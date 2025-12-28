#include "Engine.hpp"
#include <algorithm>
#include <utility>
#include <cassert>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <atomic>
#include <csignal>

using namespace Engine;

namespace {
enum class EngineState { Boot, Running, Paused, ShuttingDown, ShutdownComplete };

struct FrameStats {
    float delta = 0.0f;
    float fixedDelta = 0.0f;
    uint64_t frameIndex = 0;
    uint32_t fixedSteps = 0;
    float accumulator = 0.0f;
    double lastFrameTimeSec = 0.0;
    float fps = 0.0f;
};

struct Logger {
    std::mutex m;
    std::deque<std::string> q;
    std::atomic<int> level{0};
    void push(const std::string& s) {
        std::lock_guard<std::mutex> lk(m);
        q.push_back(s);
        if (q.size() > 1024) q.pop_front();
    }
    void flush() {
        std::lock_guard<std::mutex> lk(m);
        while (!q.empty()) {
            std::cout << q.front() << std::endl;
            q.pop_front();
        }
    }
} g_logger;

std::atomic<EngineState> g_state{EngineState::Boot};
FrameStats g_frameStats;

using TimePoint = std::chrono::high_resolution_clock::time_point;

struct TimerEntry {
    uint64_t id;
    TimePoint when;
    std::function<void()> cb;
    bool repeat;
    std::chrono::duration<double> period;
};

std::mutex g_timerMutex;
std::vector<TimerEntry> g_timers;
std::atomic<uint64_t> g_nextTimerId{1};

uint64_t addTimerInternal(std::function<void()> cb, std::chrono::duration<double> delay, bool repeat, std::chrono::duration<double> period) {
    std::lock_guard<std::mutex> lk(g_timerMutex);
    TimerEntry e;
    e.id = g_nextTimerId.fetch_add(1);
    e.when = std::chrono::high_resolution_clock::now() + delay;
    e.cb = std::move(cb);
    e.repeat = repeat;
    e.period = period;
    g_timers.push_back(std::move(e));
    return e.id;
}

void processTimers() {
    std::lock_guard<std::mutex> lk(g_timerMutex);
    if (g_timers.empty()) return;
    auto now = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < g_timers.size();) {
        if (g_timers[i].when <= now) {
            auto cb = std::move(g_timers[i].cb);
            bool rep = g_timers[i].repeat;
            auto per = g_timers[i].period;
            if (i + 1 < g_timers.size()) g_timers[i] = std::move(g_timers.back());
            g_timers.pop_back();
            try {
                if (cb) cb();
            } catch (...) {}
            if (rep) {
                addTimerInternal(std::move(cb), per, true, per);
            }
        } else {
            ++i;
        }
    }
}

void clearTimers() {
    std::lock_guard<std::mutex> lk(g_timerMutex);
    g_timers.clear();
}

void defaultSignalHandler(int sig) {
    std::ostringstream os;
    os << "Engine received signal " << sig;
    g_logger.push(os.str());
    EngineCore::RequestQuit();
}

}

void EngineCore::startWorkers() {
    std::lock_guard<std::mutex> lk(workerLifecycleMutex_);
    if (!workers_.empty()) return;
    for (int i = 0; i < config_.workerThreads; ++i) {
        workers_.emplace_back([this, i]() {
            try {
                workerThreadLoop();
            } catch (...) {
            }
        });
    }
}

void EngineCore::stopWorkers() {
    {
        std::lock_guard<std::mutex> lk(bgTasksMutex_);
        bgTasks_.clear();
    }
    bgTasksCv_.notify_all();
    for (auto& t : workers_) {
        if (t.joinable()) {
            try { t.join(); } catch (...) {}
        }
    }
    workers_.clear();
}

void EngineCore::workerThreadLoop() {
    while (!requestQuit_.load(std::memory_order_acquire)) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lk(bgTasksMutex_);
            bgTasksCv_.wait(lk, [this]() {
                return requestQuit_.load(std::memory_order_acquire) || !bgTasks_.empty();
            });
            if (requestQuit_.load(std::memory_order_acquire) && bgTasks_.empty()) return;
            if (!bgTasks_.empty()) {
                task = std::move(bgTasks_.front());
                bgTasks_.pop_front();
            }
        }
        if (!task) continue;
        activeBgTasks_.fetch_add(1, std::memory_order_relaxed);
        try { task(); } catch (...) {}
        activeBgTasks_.fetch_sub(1, std::memory_order_relaxed);
        workerLifecycleCv_.notify_all();
    }
}

void EngineCore::run() {
    if (running_.exchange(true, std::memory_order_acq_rel)) return;
    startWorkers();
    std::signal(SIGINT, &defaultSignalHandler);
    std::signal(SIGTERM, &defaultSignalHandler);
    g_state.store(EngineState::Running, std::memory_order_release);
    g_frameStats = {};

    auto lastTime = std::chrono::high_resolution_clock::now();
    std::chrono::nanoseconds frameBudget(0);
    if (config_.targetFps > 0) frameBudget = std::chrono::nanoseconds(1000000000 / config_.targetFps);

    while (!shouldQuit() && !Platform::Window::ShouldClose()) {
        Platform::Window::PollEvents();
        Time::Update();
        g_frameStats.delta = Time::Delta();
        g_frameStats.fixedDelta = config_.fixedStep;
        g_frameStats.accumulator += g_frameStats.delta;
        g_frameStats.fixedSteps = 0;

        processTimers();

        if (g_state.load(std::memory_order_acquire) == EngineState::Running) {
            int steps = 0;
            while (g_frameStats.accumulator >= config_.fixedStep && steps < config_.maxFixedStepsPerFrame) {
                {
                    std::lock_guard<std::mutex> lk(cbMutex_);
                    for (auto& cb : fixedStepCbs_) {
                        try { cb(config_.fixedStep); } catch (...) {}
                    }
                }
                SystemManager::FixedUpdateAll(config_.fixedStep);
                Physics::PhysicsSystem::Step(config_.fixedStep);
                g_frameStats.accumulator -= config_.fixedStep;
                ++g_frameStats.fixedSteps;
                ++steps;
            }
        }

        {
            std::deque<std::function<void()>> tasks;
            {
                std::lock_guard<std::mutex> lk(mainTasksMutex_);
                tasks.swap(mainThreadTasks_);
            }
            for (auto& t : tasks) {
                try { t(); } catch (...) {}
            }
        }

        if (g_state.load(std::memory_order_acquire) == EngineState::Running) {
            {
                std::lock_guard<std::mutex> lk(cbMutex_);
                for (auto& cb : updateCbs_) {
                    try { cb(g_frameStats.delta); } catch (...) {}
                }
            }
            SystemManager::UpdateAll(g_frameStats.delta);
            Scene::SceneManager::UpdateActive(g_frameStats.delta);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        Graphics::Renderer::BeginFrame();
        SystemManager::RenderAll();
        Scene::SceneManager::RenderActive();
        Graphics::Renderer::EndFrame();
        Graphics::Renderer::Present();
        Platform::Window::SwapBuffers();
        Input::Manager::EndFrame();

        ++g_frameStats.frameIndex;
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> ft = now - lastTime;
        g_frameStats.lastFrameTimeSec = ft.count();
        if (ft.count() > 0.0) g_frameStats.fps = static_cast<float>(1.0 / ft.count());
        lastTime = now;

        if (frameBudget.count() > 0) {
            auto elapsed = std::chrono::high_resolution_clock::now() - now;
            if (elapsed < frameBudget) std::this_thread::sleep_for(frameBudget - elapsed);
        }
    }

    running_.store(false, std::memory_order_release);
    g_state.store(EngineState::ShuttingDown, std::memory_order_release);

    requestQuit_.store(true, std::memory_order_release);
    bgTasksCv_.notify_all();

    shutdown();
}

void EngineCore::requestQuit() {
    requestQuit_.store(true, std::memory_order_release);
    bgTasksCv_.notify_all();
}

bool EngineCore::shouldQuit() const {
    return requestQuit_.load(std::memory_order_acquire);
}

void EngineCore::shutdown() {
    requestQuit_.store(true, std::memory_order_release);

    {
        std::unique_lock<std::mutex> lk(workerLifecycleMutex_);
        workerLifecycleCv_.wait_for(lk, std::chrono::milliseconds(500), [this]() {
            return activeBgTasks_.load(std::memory_order_acquire) == 0;
        });
    }

    {
        std::lock_guard<std::mutex> lk(cbMutex_);
        for (auto& cb : shutdownCbs_) {
            try { cb(); } catch (...) {}
        }
    }

    {
        std::lock_guard<std::mutex> lk(mainTasksMutex_);
        while (!mainThreadTasks_.empty()) {
            try { mainThreadTasks_.front()(); } catch (...) {}
            mainThreadTasks_.pop_front();
        }
    }

    stopWorkers();
    clearTimers();

    SystemManager::Shutdown();
    Scene::SceneManager::Shutdown();
    Graphics::Renderer::Shutdown();
    Platform::Window::Destroy();

    g_logger.push("Engine shutdown complete");
    g_logger.flush();

    g_state.store(EngineState::ShutdownComplete, std::memory_order_release);
}

void EngineCore::performShutdown() {
    shutdown();
}

void EngineCore::registerFixedCallback(std::function<void(float)> cb) {
    std::lock_guard<std::mutex> lk(cbMutex_);
    fixedStepCbs_.push_back(std::move(cb));
}

void EngineCore::registerUpdateCallback(std::function<void(float)> cb) {
    std::lock_guard<std::mutex> lk(cbMutex_);
    updateCbs_.push_back(std::move(cb));
}

void EngineCore::registerShutdownCallback(std::function<void()> cb) {
    std::lock_guard<std::mutex> lk(cbMutex_);
    shutdownCbs_.push_back(std::move(cb));
}

void EngineCore::registerSystem(std::unique_ptr<SystemManager::ISystem> sys, int priority) {
    std::lock_guard<std::mutex> lk(systemMutex_);
    systems_.push_back(SystemEntry{std::move(sys), priority});
    std::sort(systems_.begin(), systems_.end(), [](const SystemEntry& a, const SystemEntry& b){
        return a.priority < b.priority;
    });
    for (auto& entry : systems_) {
        if (entry.sys) {
            try { entry.sys->OnInit(); } catch (...) {}
        }
    }
}

void EngineCore::enqueueMainThreadTask(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lk(mainTasksMutex_);
        mainThreadTasks_.push_back(std::move(task));
    }
    mainTasksCv_.notify_one();
}

void EngineCore::enqueueBackgroundTask(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lk(bgTasksMutex_);
        bgTasks_.push_back(std::move(task));
    }
    bgTasksCv_.notify_one();
}

void EngineCore::waitForBackgroundTasks() {
    std::unique_lock<std::mutex> lk(workerLifecycleMutex_);
    workerLifecycleCv_.wait(lk, [this]() {
        return activeBgTasks_.load(std::memory_order_acquire) == 0 && bgTasks_.empty();
    });
}
