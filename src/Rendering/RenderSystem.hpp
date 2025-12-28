#pragma once

#include “../../Scene/SceneManager.hpp”
#include “../../ECS/Components/Transform.hpp”
#include “../../ECS/Components/MeshRenderer.hpp”
#include “../../ECS/Components/Camera.hpp”
#include <vector>
#include <unordered_map>

namespace Titan::Rendering {

using namespace ECS::Components;

struct RenderCommand {
uint32_t meshId;
uint32_t materialId;
Vec3 position;
Quaternion rotation;
Vec3 scale;
Color tint;
int layer;
};

struct CameraData {
Vec3 position;
Quaternion rotation;
ProjectionType projectionType;
float fov;
float nearClip;
float farClip;
float orthoSize;
float aspectRatio;
int renderOrder;
};

class RenderSystem : public Scene::ISystem {
public:
RenderSystem() = default;

```
void OnInit() override {
    InitOpenGL();
}

void OnShutdown() override {
    CleanupOpenGL();
}

void OnRender() override {
    auto scene = Scene::SceneManager::Get().GetActiveScene();
    if (!scene || !scene->world) return;
    
    CollectCameras(scene->world.get());
    CollectRenderCommands(scene->world.get());
    
    for (auto& camera : cameras) {
        RenderWithCamera(camera);
    }
    
    cameras.clear();
    renderCommands.clear();
}
```

private:
std::vector<CameraData> cameras;
std::vector<RenderCommand> renderCommands;

```
uint32_t quadVAO = 0;
uint32_t quadVBO = 0;
uint32_t quadEBO = 0;
uint32_t instanceVBO = 0;

void InitOpenGL() {
    CreateQuadGeometry();
}

void CleanupOpenGL() {
    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
    if (quadEBO) glDeleteBuffers(1, &quadEBO);
    if (instanceVBO) glDeleteBuffers(1, &instanceVBO);
}

void CreateQuadGeometry() {
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
    };
    
    uint32_t indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);
    glGenBuffers(1, &instanceVBO);
    
    glBindVertexArray(quadVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    
    glBindVertexArray(0);
}

void CollectCameras(ECS::World* world) {
    world->Each<Transform, Camera>([this](ECS::Entity entity, Transform& t, Camera& cam) {
        CameraData camData;
        camData.position = t.position;
        camData.rotation = t.rotation;
        camData.projectionType = cam.projectionType;
        camData.fov = cam.fieldOfView;
        camData.nearClip = cam.nearClip;
        camData.farClip = cam.farClip;
        camData.orthoSize = cam.orthographicSize;
        camData.aspectRatio = cam.aspectRatio;
        camData.renderOrder = cam.renderOrder;
        
        cameras.push_back(camData);
    });
    
    std::sort(cameras.begin(), cameras.end(), 
        [](const CameraData& a, const CameraData& b) {
            return a.renderOrder < b.renderOrder;
        });
}

void CollectRenderCommands(ECS::World* world) {
    world->Each<Transform, MeshRenderer>([this](ECS::Entity entity, Transform& t, MeshRenderer& mr) {
        if (!mr.visible) return;
        
        RenderCommand cmd;
        cmd.meshId = mr.meshId;
        cmd.materialId = mr.materialId;
        cmd.position = t.position;
        cmd.rotation = t.rotation;
        cmd.scale = t.scale;
        cmd.tint = mr.tint;
        cmd.layer = mr.renderLayer;
        
        renderCommands.push_back(cmd);
    });
    
    std::sort(renderCommands.begin(), renderCommands.end(),
        [](const RenderCommand& a, const RenderCommand& b) {
            if (a.layer != b.layer) return a.layer < b.layer;
            if (a.materialId != b.materialId) return a.materialId < b.materialId;
            return a.meshId < b.meshId;
        });
}

void RenderWithCamera(const CameraData& camera) {
    SetupCameraMatrices(camera);
    
    uint32_t currentMaterial = 0;
    uint32_t currentMesh = 0;
    
    for (const auto& cmd : renderCommands) {
        if (cmd.materialId != currentMaterial) {
            BindMaterial(cmd.materialId);
            currentMaterial = cmd.materialId;
        }
        
        if (cmd.meshId != currentMesh) {
            BindMesh(cmd.meshId);
            currentMesh = cmd.meshId;
        }
        
        SetModelMatrix(cmd.position, cmd.rotation, cmd.scale);
        SetTint(cmd.tint);
        
        DrawMesh(cmd.meshId);
    }
}

void SetupCameraMatrices(const CameraData& camera) {
}

void BindMaterial(uint32_t materialId) {
}

void BindMesh(uint32_t meshId) {
}

void SetModelMatrix(const Vec3& pos, const Quaternion& rot, const Vec3& scale) {
}

void SetTint(const Color& tint) {
}

void DrawMesh(uint32_t meshId) {
}
```

};

}
