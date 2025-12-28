#pragma once

#include “../Scene/SceneManager.hpp”
#include “../Platform/Input.hpp”
#include “CharacterController.hpp”
#include “PhysicsWorld.hpp”

namespace Titan::Physics {

class FPSGameSystem : public Scene::ISystem {
public:
void OnInit() override {
auto scene = Scene::SceneManager::Get().GetActiveScene();
if (!scene || !scene->world) return;

```
    player = scene->world->CreateEntity();
    
    auto& playerTransform = scene->world->AddComponent<Transform>(player);
    playerTransform.position = Vec3(0, 5, 0);
    
    auto& playerRb = scene->world->AddComponent<Rigidbody>(player);
    playerRb.type = RigidbodyType::Dynamic;
    playerRb.mass = 80.0f;
    playerRb.useGravity = true;
    playerRb.freezeRotation = true;
    
    auto& playerCol = scene->world->AddComponent<Collider>(player);
    playerCol = Collider::Capsule(0.4f, 1.8f);
    
    auto& camera = scene->world->AddComponent<Camera>(player);
    camera.SetPerspective(90.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    camera.isPrimary = true;
    
    auto& renderer = scene->world->AddComponent<MeshRenderer>(player);
    renderer.visible = false;
    
    CreateGround(scene->world.get());
    SpawnCubes(scene->world.get(), 20);
}

void OnUpdate(float dt) override {
    auto scene = Scene::SceneManager::Get().GetActiveScene();
    if (!scene || !scene->world) return;
    
    if (!scene->world->IsEntityValid(player)) return;
    
    auto& transform = scene->world->GetComponent<Transform>(player);
    auto& rb = scene->world->GetComponent<Rigidbody>(player);
    
    Vec3 moveInput = Vec3::Zero();
    if (Platform::Input::IsKeyDown(Platform::Input::Key::W)) moveInput.z += 1.0f;
    if (Platform::Input::IsKeyDown(Platform::Input::Key::S)) moveInput.z -= 1.0f;
    if (Platform::Input::IsKeyDown(Platform::Input::Key::A)) moveInput.x -= 1.0f;
    if (Platform::Input::IsKeyDown(Platform::Input::Key::D)) moveInput.x += 1.0f;
    
    Vec2 mouseInput;
    mouseInput.x = (float)Platform::Input::GetMouseDeltaX();
    mouseInput.y = (float)Platform::Input::GetMouseDeltaY();
    
    bool jump = Platform::Input::IsKeyPressed(Platform::Input::Key::Space);
    bool sprint = Platform::Input::IsKeyDown(Platform::Input::Key::LeftShift);
    
    fpsController.Update(transform, rb, dt, moveInput, mouseInput, jump, sprint);
    
    if (Platform::Input::IsMouseButtonPressed(Platform::Input::MouseButton::Left)) {
        ShootRaycast(transform.position, transform.Forward());
    }
    
    if (Platform::Input::IsKeyPressed(Platform::Input::Key::E)) {
        SpawnCube(scene->world.get(), transform.position + transform.Forward() * 2.0f);
    }
}
```

private:
Entity player;
FirstPersonController fpsController;

```
void CreateGround(World* world) {
    auto ground = world->CreateEntity();
    
    auto& t = world->AddComponent<Transform>(ground);
    t.position = Vec3(0, -1, 0);
    t.scale = Vec3(50, 1, 50);
    
    auto& col = world->AddComponent<Collider>(ground);
    col = Collider::Box(Vec3(50, 1, 50));
    
    auto& rb = world->AddComponent<Rigidbody>(ground);
    rb.type = RigidbodyType::Static;
    
    auto& renderer = world->AddComponent<MeshRenderer>(ground);
    renderer.tint = Color(0.3f, 0.3f, 0.3f);
}

void SpawnCubes(World* world, int count) {
    for (int i = 0; i < count; ++i) {
        float x = (float)(rand() % 20 - 10);
        float z = (float)(rand() % 20 - 10);
        float y = 5.0f + i * 2.0f;
        
        SpawnCube(world, Vec3(x, y, z));
    }
}

void SpawnCube(World* world, const Vec3& position) {
    auto cube = world->CreateEntity();
    
    auto& t = world->AddComponent<Transform>(cube);
    t.position = position;
    t.scale = Vec3(1, 1, 1);
    
    auto& col = world->AddComponent<Collider>(cube);
    col = Collider::Box(Vec3(1, 1, 1));
    
    auto& rb = world->AddComponent<Rigidbody>(cube);
    rb.type = RigidbodyType::Dynamic;
    rb.mass = 10.0f;
    rb.useGravity = true;
    
    auto& renderer = world->AddComponent<MeshRenderer>(cube);
    renderer.tint = Color(
        (float)(rand() % 100) / 100.0f,
        (float)(rand() % 100) / 100.0f,
        (float)(rand() % 100) / 100.0f
    );
}

void ShootRaycast(const Vec3& origin, const Vec3& direction) {
    auto hit = PhysicsWorld::Get().Raycast(origin, direction, 100.0f);
    
    if (hit.hit) {
        auto scene = Scene::SceneManager::Get().GetActiveScene();
        if (!scene || !scene->world) return;
        
        if (scene->world->HasComponent<Rigidbody>(hit.entity)) {
            auto& rb = scene->world->GetComponent<Rigidbody>(hit.entity);
            
            Vec3 force = direction.Normalized() * 500.0f;
            rb.AddImpulse(force);
        }
    }
}
```

};

}
