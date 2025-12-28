#pragma once

#include “../Scene/SceneManager.hpp”
#include “../ECS/Components/Transform.hpp”
#include “../ECS/Components/MeshRenderer.hpp”
#include “../ECS/Components/Camera.hpp”
#include “../Graphics/Renderer.hpp”
#include “../Graphics/DebugRenderer.hpp”
#include “../Graphics/Mesh.hpp”
#include “../Graphics/Material.hpp”
#include “../Platform/Window.hpp”

namespace Titan::Rendering {

using namespace ECS::Components;
using namespace Graphics;

class RenderSystem : public Scene::ISystem {
public:
RenderSystem()
: defaultMesh(nullptr)
, defaultMaterial(nullptr) {}

```
void OnInit() override {
    Renderer::Get().Init();
    DebugRenderer::Get().Init();

    defaultMesh = Mesh::CreateCube();
    defaultMaterial = MaterialPresets::CreateLit(Color(0.8f, 0.8f, 0.8f));

    MeshLibrary::Load("cube", "");
    defaultMesh = MeshLibrary::Get("cube");
    if (!defaultMesh) {
        defaultMesh = Mesh::CreateCube();
    }

    Renderer::Get().SetClearColor(0.1f, 0.1f, 0.12f, 1.0f);

    Logger::Info("RenderSystem initialized");
}

void OnShutdown() override {
    if (defaultMesh) {
        delete defaultMesh;
        defaultMesh = nullptr;
    }

    if (defaultMaterial) {
        delete defaultMaterial;
        defaultMaterial = nullptr;
    }

    MaterialLibrary::Clear();
    MeshLibrary::Clear();
    ShaderLibrary::Clear();

    DebugRenderer::Get().Shutdown();
    Renderer::Get().Shutdown();

    Logger::Info("RenderSystem shutdown");
}

void OnUpdate(float dt) override {
    DebugRenderer::Get().Update(dt);
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
        Renderer::Get().BeginScene(*primaryCamera, *cameraTransform);

        scene->world->Each<Transform, MeshRenderer>([this](ECS::Entity e, Transform& t, MeshRenderer& mr) {
            if (!mr.visible) return;

            Mesh* mesh = defaultMesh;
            if (!mr.meshPath.empty()) {
                mesh = MeshLibrary::GetOrCreate(mr.meshPath, mr.meshPath);
                if (!mesh) mesh = defaultMesh;
            }

            Material* material = defaultMaterial;
            if (mr.materialId != 0) {
                material = reinterpret_cast<Material*>(mr.materialId);
            }
            
            if (!material) {
                material = defaultMaterial;
            }

            if (material && mesh) {
                material->GetProperties().albedo = mr.tint;
                Renderer::Get().Submit(mesh, material, t);
            }
        });

        Mat4 view = Renderer::Get().GetViewMatrix();
        Mat4 projection = Renderer::Get().GetProjectionMatrix();

        DebugRenderer::Get().Render(view, projection);

        Renderer::Get().EndScene();

        auto& stats = Renderer::Get().GetStats();
        if (stats.frameCount % 60 == 0) {
            Logger::Info("Renderer Stats - DrawCalls: " + std::to_string(stats.drawCalls) +
                       " Triangles: " + std::to_string(stats.triangles) +
                       " Batches: " + std::to_string(stats.batches) +
                       " Culled: " + std::to_string(stats.entitiesCulled));
        }
    }

    Renderer::Get().EndFrame();
}
```

private:
Mesh* defaultMesh;
Material* defaultMaterial;
};

}
