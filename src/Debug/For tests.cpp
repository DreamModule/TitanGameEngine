#include “Debug/DebugSystem.hpp”
#include “Platform/Window.hpp”
#include “Input/InputManager.hpp”
#include <gl/GL.h>

void GameUpdate(float dt) {
PROFILE_FUNCTION();

```
Titan::Debug::TrackMetric("DeltaTime", dt * 1000.0f);
```

}

void GameRender() {
PROFILE_FUNCTION();

```
{
    PROFILE_SCOPE("Scene Rendering");
}

Titan::Debug::Begin();

static Titan::Debug::Panel* gamePanel = nullptr;
if (!gamePanel) {
    gamePanel = Titan::Debug::CreatePanel("Game Info", 320, 10, 280, 180);
    Titan::Debug::SetPanelCallback(gamePanel, []() {
        auto style = Titan::Debug::GetStyle();
        int x = 320 + style.padding;
        int y = 10 + style.fontSize + style.padding * 3;
        int lineH = style.fontSize + style.margin;
        
        char buf[128];
        sprintf(buf, "Delta Time: %.2f ms", Titan::Debug::GetMetric("DeltaTime"));
        Titan::Debug::Text(x, y, buf);
        y += lineH * 2;
        
        static bool showColliders = false;
        Titan::Debug::Checkbox(x, y, "Show Colliders", &showColliders);
        y += lineH + style.padding;
        
        static float volume = 0.5f;
        Titan::Debug::Slider(x, y, 250, "Volume:", &volume, 0.0f, 1.0f);
        y += lineH + style.padding;
        
        if (Titan::Debug::Button(x, y, 150, 25, "Reset Stats")) {
            Titan::Debug::ResetProfileData();
            Titan::Debug::LogInfo("Stats reset");
        }
    });
}

Titan::Debug::End();
```

}

int main() {
Titan::Platform::Window::CreateInfo winInfo;
winInfo.width = 1280;
winInfo.height = 720;
winInfo.title = “Titan Engine”;
Titan::Platform::Window::Create(winInfo);

```
Titan::Debug::Init();
Titan::Debug::ShowStatsWindow(true);
Titan::Debug::ShowLogWindow(false);
Titan::Debug::ShowProfilerWindow(false);

Titan::Debug::LogInfo("Engine started");

Titan::Platform::Window::SetEventCallback([](const auto& event) {
    switch (event.type) {
        case Titan::Platform::Window::EventType::KeyPress:
            Titan::Input::Manager::OnKeyDown(event.key.keycode);
            break;
        case Titan::Platform::Window::EventType::KeyRelease:
            Titan::Input::Manager::OnKeyUp(event.key.keycode);
            break;
        case Titan::Platform::Window::EventType::MouseMove:
            Titan::Input::Manager::OnMouseMove(event.mouseMove.x, event.mouseMove.y);
            break;
    }
});

while (!Titan::Platform::Window::ShouldClose()) {
    Titan::Platform::Window::PollEvents();
    Titan::Input::Manager::EndFrame();
    
    if (Titan::Input::Manager::IsKeyPressed(VK_F1)) {
        Titan::Debug::SetVisible(!Titan::Debug::IsVisible());
    }
    
    if (Titan::Input::Manager::IsKeyPressed(VK_F2)) {
        bool visible = !Titan::Debug::IsVisible();
        Titan::Debug::ShowLogWindow(visible);
    }
    
    if (Titan::Input::Manager::IsKeyPressed(VK_F3)) {
        bool visible = !Titan::Debug::IsVisible();
        Titan::Debug::ShowProfilerWindow(visible);
    }
    
    float dt = 0.016f;
    
    GameUpdate(dt);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    
    GameRender();
    
    Titan::Platform::Window::SwapBuffers();
}

Titan::Debug::Shutdown();
Titan::Platform::Window::Destroy();

return 0;
```

}

class Player {
public:
void Update(float dt) {
PROFILE_SCOPE(“Player::Update”);

```
    if (health <= 0) {
        Titan::Debug::LogError("Player died");
    }
}

void TakeDamage(float damage) {
    health -= damage;
    Titan::Debug::LogWarning("Player took %.1f damage (HP: %.1f)", damage, health);
    Titan::Debug::TrackMetric("PlayerHealth", health);
}
```

private:
float health = 100.0f;
};

class Renderer {
public:
void RenderScene() {
PROFILE_FUNCTION();

```
    {
        PROFILE_SCOPE("Terrain");
    }
    
    {
        PROFILE_SCOPE("Characters");
    }
    
    {
        PROFILE_SCOPE("Effects");
    }
    
    Titan::Debug::TrackMetric("DrawCalls", drawCallCount);
    Titan::Debug::TrackMetric("Triangles", triangleCount);
}
```

private:
float drawCallCount = 0;
float triangleCount = 0;
};
