#pragma once

#include “../Scene/SceneManager.hpp”
#include “../ECS/Components/Transform.hpp”
#include “../ECS/Components/MeshRenderer.hpp”
#include “../ECS/Components/Camera.hpp”
#include “../Graphics/Renderer.hpp”
#include “../Platform/Window.hpp”

namespace Titan::Rendering {

using namespace ECS::Components;
using namespace Graphics;

class RenderSystem : public Scene::ISystem {
public:
void OnInit() override {
Renderer::Get().Init();

```
    cubeMeshId = Renderer::Get().CreateCubeMesh();
    
    Renderer::Get().SetClearColor(0.1f, 0.1f, 0.12f, 1.0f);
}

void OnShutdown() override {
    Renderer::Get().Shutdown();
}

void OnRender() override {
    auto scene = Scene::SceneManager::Get().GetActiveScene();
    if (!scene || !scene->world) return;
    
    uint32_t width = Platform::Window::GetWidth();
    uint32_t height = Platform::Window::GetHeight();
    
    Renderer::Get().SetViewport(0, 0, width, height);
    Renderer::Get().BeginFrame();
    
    Camera* primaryCamera = nullptr;
    Transform* cameraTransform = nullptr;
    
    scene->world->Each<Transform, Camera>([&](ECS::Entity e, Transform& t, Camera& cam) {
        if (cam.isPrimary || !primaryCamera) {
            primaryCamera = &cam;
            cameraTransform = &t;
        }
    });
    
    if (primaryCamera && cameraTransform) {
        Mat4 view = Mat4::LookAt(
            cameraTransform->position,
            cameraTransform->position + cameraTransform->Forward(),
            Vec3::Up()
        );
        
        Mat4 projection;
        if (primaryCamera->projectionType == ProjectionType::Perspective) {
            float aspect = (float)width / (float)height;
            projection = Mat4::Perspective(
                primaryCamera->fieldOfView,
                aspect,
                primaryCamera->nearClip,
                primaryCamera->farClip
            );
        } else {
            float halfWidth = primaryCamera->orthographicSize * primaryCamera->aspectRatio * 0.5f;
            float halfHeight = primaryCamera->orthographicSize * 0.5f;
            projection = Mat4::Orthographic(
                -halfWidth, halfWidth,
                -halfHeight, halfHeight,
                primaryCamera->nearClip,
                primaryCamera->farClip
            );
        }
        
        Renderer::Get().BeginScene(view, projection);
        
        scene->world->Each<Transform, MeshRenderer>([this](ECS::Entity e, Transform& t, MeshRenderer& mr) {
            if (!mr.visible) return;
            
            uint32_t meshId = cubeMeshId;
            uint32_t textureId = 0;
            
            Renderer::Get().Submit(
                t.position,
                t.rotation,
                t.scale,
                meshId,
                textureId,
                mr.tint
            );
        });
        
        Renderer::Get().EndScene();
    }
    
    Renderer::Get().EndFrame();
}
```

private:
uint32_t cubeMeshId = 0;
};

}
