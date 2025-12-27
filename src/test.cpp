#include "Platform/Window.hpp"
#include "Graphics/GraphicsDevice.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/SceneSerializer.hpp"
#include "ECS/World.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/Sprite.hpp"
#include "Core/Systems/RenderEntitySystem.hpp"
#include "Input/InputManager.hpp"
#include "Input/Joystick.hpp"
#include "Debug/Debug.hpp"
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

    auto ePlayer = scene->world->CreateEntity();
    Titan::ECS::Components::Transform pt;
    pt.x = 640; pt.y = 360; pt.sx = 1.0f; pt.sy = 1.0f; pt.rot = 0.0f;
    scene->world->transforms.emplace(ePlayer, pt);
    Titan::ECS::Components::Sprite ps;
    ps.path = "assets/player.png";
    scene->world->sprites.emplace(ePlayer, ps);

    auto eEnemy = scene->world->CreateEntity();
    Titan::ECS::Components::Transform et;
    et.x = 300; et.y = 200; et.sx = 1.0f; et.sy = 1.0f; et.rot = 0.0f;
    scene->world->transforms.emplace(eEnemy, et);
    Titan::ECS::Components::Sprite es;
    es.path = "assets/enemy.png";
    scene->world->sprites.emplace(eEnemy, es);

    Titan::Core::Systems::RenderEntitySystem renderSystem;

    auto& vj = Titan::Input::Joystick::GetVirtual();
    if (Titan::Platform::Window::IsAndroid()) vj.SetEnabled(true);
    else vj.SetEnabled(false);

    auto last = std::chrono::high_resolution_clock::now();
    int fpsCounter = 0;
    float fpsTimer = 0.0f;
    int fps = 0;

    while (!Titan::Platform::Window::ShouldClose()) {
        Titan::Platform::Window::PollEvents();

        Titan::Input::Joystick::Update();

        bool mouseDown = Titan::Input::Manager::IsKeyDown(VK_LBUTTON);
        int mx = Titan::Input::Manager::GetMouseX();
        int my = Titan::Input::Manager::GetMouseY();
        if (mouseDown && !vj.enabled) {
            // simulate a virtual touch for testing with mouse even if virtual joystick disabled
            Titan::Input::Joystick::SimulateVirtualTouchDown((float)mx, (float)my);
        } else if (mouseDown && vj.enabled) {
            Titan::Input::Joystick::SimulateVirtualTouchMove((float)mx, (float)my);
        } else {
            Titan::Input::Joystick::SimulateVirtualTouchUp();
        }

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

        if (Titan::Input::Manager::IsKeyPressed('J')) {
            vj.SetEnabled(!vj.enabled);
        }
        if (Titan::Input::Manager::IsKeyPressed('K')) {
            vj.SetRadius(vj.radius + 8.0f);
        }
        if (Titan::Input::Manager::IsKeyPressed('L')) {
            vj.SetRadius(std::max(8.0f, vj.radius - 8.0f));
        }
        if (Titan::Input::Manager::IsKeyPressed(VK_ESCAPE)) break;

        float ax = 0.0f;
        float ay = 0.0f;
        if (Titan::Input::Joystick::IsControllerConnected(0)) {
            ax = Titan::Input::Joystick::GetLeftAxisX(0);
            ay = -Titan::Input::Joystick::GetLeftAxisY(0);
        } else if (vj.enabled) {
            ax = vj.stickX;
            ay = -vj.stickY;
        } else {
            if (Titan::Input::Manager::IsKeyDown(0x25)) ax -= 1.0f;
            if (Titan::Input::Manager::IsKeyDown(0x27)) ax += 1.0f;
            if (Titan::Input::Manager::IsKeyDown(0x26)) ay -= 1.0f;
            if (Titan::Input::Manager::IsKeyDown(0x28)) ay += 1.0f;
        }

        float speed = 320.0f;
        auto &ptref = scene->world->transforms[ePlayer];
        ptref.x += ax * speed * delta;
        ptref.y += ay * speed * delta;

        if (Titan::Input::Joystick::IsControllerConnected(0)) {
            auto cs = Titan::Input::Joystick::GetController(0);
            if (cs.buttons) {
                // placeholder: if any button pressed, toggle enemy visibility by moving it
                scene->world->transforms[eEnemy].x = 400 + (cs.buttons % 64);
            }
        }

        scene->world->transforms[eEnemy].rot += 90.0f * delta;

        Titan::Debug::Begin();

        char buf[128];
        sprintf(buf, "FPS: %d", fps);
        Titan::Debug::Text(10, 18, buf);

        sprintf(buf, "Virtual J: %s  Radius: %.1f", vj.enabled ? "ON" : "OFF", vj.radius);
        Titan::Debug::Text(10, 36, buf);

        sprintf(buf, "Joystick AX: %.2f AY: %.2f", ax, ay);
        Titan::Debug::Text(10, 54, buf);

        if (Titan::Debug::Button(10, 80, 180, 28, vj.enabled ? "Disable Virtual Joy" : "Enable Virtual Joy")) {
            vj.SetEnabled(!vj.enabled);
        }
        if (Titan::Debug::Button(10, 116, 140, 28, "Increase Radius")) {
            vj.SetRadius(vj.radius + 8.0f);
        }
        if (Titan::Debug::Button(160, 116, 140, 28, "Decrease Radius")) {
            vj.SetRadius(std::max(8.0f, vj.radius - 8.0f));
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
