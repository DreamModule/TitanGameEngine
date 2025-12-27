#include "Platform/Window.hpp"
#include "Graphics/GraphicsDevice.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/SceneSerializer.hpp"
#include "ECS/World.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/Sprite.hpp"
#include "Core/Systems/RenderEntitySystem.hpp"
#include "Core/Systems/PlatformPollSystem.hpp"
#include "Input/InputManager.hpp"
#include "Debug/Debug.hpp"
#include "Assets/Loader.hpp"
#include <chrono>
#include <thread>
#include <memory>
#include <string>

extern "C" void RenderEntitySystem_SetSceneManager(Titan::Scene::SceneManager* m);

int main() {
    if (!Titan::Platform::Window::Create(1280, 720, "Titan - Test Game")) return -1;
    if (!Titan::Graphics::Device::Init(Titan::Platform::Window::GetHWND(), Titan::Platform::Window::GetHDC())) {
        Titan::Platform::Window::Destroy();
        return -1;
    }

    Titan::Debug::Init();

    auto sceneManager = std::make_unique<Titan::Scene::SceneManager>();
    RenderEntitySystem_SetSceneManager(sceneManager.get());
    auto scene = sceneManager->CreateScene("test");
    sceneManager->ActivateScene("test");

    auto e1 = scene->world->CreateEntity();
    Titan::ECS::Components::Transform t1;
    t1.x = 640; t1.y = 360; t1.sx = 1.0f; t1.sy = 1.0f;
    scene->world->transforms.emplace(e1, t1);
    Titan::ECS::Components::Sprite s1;
    s1.path = "assets/player.png";
    scene->world->sprites.emplace(e1, s1);

    auto e2 = scene->world->CreateEntity();
    Titan::ECS::Components::Transform t2;
    t2.x = 200; t2.y = 200; t2.sx = 1.0f; t2.sy = 1.0f;
    scene->world->transforms.emplace(e2, t2);
    Titan::ECS::Components::Sprite s2;
    s2.path = "assets/enemy.png";
    scene->world->sprites.emplace(e2, s2);

    Titan::Core::Systems::RenderEntitySystem renderSystem;
    Titan::Core::Systems::PlatformPollSystem pollSystem;

    auto last = std::chrono::high_resolution_clock::now();
    float rot = 0.0f;
    bool showDemo = true;
    int fpsCounter = 0;
    float fpsTimer = 0.0f;
    int fps = 0;

    while (!Titan::Platform::Window::ShouldClose()) {
        pollSystem.Update(*(new Titan::Core::FrameContext())); // poll events and potentially set shouldClose

        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> dt = now - last;
        last = now;
        float delta = dt.count();

        fpsTimer += delta;
        fpsCounter++;
        if (fpsTimer >= 1.0f) {
            fps = fpsCounter;
            fpsCounter = 0;
            fpsTimer -= 1.0f;
        }

        if (Titan::Input::Manager::IsKeyPressed('S')) {
            Titan::Scene::SceneSerializer::SaveScene(scene, "autosave.scene");
        }
        if (Titan::Input::Manager::IsKeyPressed('L')) {
            Titan::Scene::SceneSerializer::LoadScene(scene, "autosave.scene");
        }
        if (Titan::Input::Manager::IsKeyPressed(VK_ESCAPE)) break;

        if (Titan::Input::Manager::IsKeyDown(0x25)) scene->world->transforms[e1].x -= 300.0f * delta;
        if (Titan::Input::Manager::IsKeyDown(0x27)) scene->world->transforms[e1].x += 300.0f * delta;
        if (Titan::Input::Manager::IsKeyDown(0x26)) scene->world->transforms[e1].y -= 300.0f * delta;
        if (Titan::Input::Manager::IsKeyDown(0x28)) scene->world->transforms[e1].y += 300.0f * delta;

        rot += 60.0f * delta;
        scene->world->transforms[e2].rot = rot;

        Titan::Debug::Begin();

        char buf[128];
        sprintf(buf, "FPS: %d", fps);
        Titan::Debug::Text(10, 18, buf);
        Titan::Debug::Text(10, 36, "Arrows: move player   S: save scene   L: load scene   Esc: quit");

        if (Titan::Debug::Button(10, 60, 140, 28, showDemo ? "Hide Demo Info" : "Show Demo Info")) {
            showDemo = !showDemo;
        }

        if (showDemo) {
            Titan::Debug::Rect(10, 100, 300, 120, 0.2f, 0.2f, 0.2f, 0.9f);
            Titan::Debug::Text(16, 116, "Demo Scene:");
            Titan::Debug::Text(16, 134, " - Player: controllable with arrows");
            Titan::Debug::Text(16, 152, " - Enemy: rotates automatically");
            Titan::Debug::Text(16, 170, " - S/L: save/load scene to autosave.scene");
        }

        Titan::Debug::End();

        Titan::Core::FrameContext fc;
        fc.dt = delta;
        fc.engine = nullptr;

        renderSystem.Update(fc);

        Titan::Platform::Window::SwapBuffers();

        Titan::Input::Manager::EndFrame();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Titan::Debug::Shutdown();
    Titan::Graphics::Device::Shutdown();
    Titan::Platform::Window::Destroy();

    return 0;
}
