# Titan Engine - Game Development Guide

## Quick Start

### Build Command (MinGW)
```cmd
g++ -std=c++17 -O2 -I. -Isrc -D_WIN32 -DNOMINMAX your_game.cpp -o Game.exe -lopengl32 -luser32 -lgdi32 -lbcrypt -lws2_32
```

### Build Command (MSVC)
```cmd
cl /std:c++17 /O2 /I. /Isrc /D_WIN32 /DNOMINMAX your_game.cpp /Fe:Game.exe /link opengl32.lib user32.lib gdi32.lib bcrypt.lib ws2_32.lib
```

## API Reference
чо
### Window
```cpp
Window::Create("Game Title", 1280, 720);
Window::SetFullscreen(true);
Window::Close();
bool running = Window::IsOpen();
```

### Input
```cpp
Input::MapAction("Jump", KeyCode::Space);
Input::MapAction("Shoot", KeyCode::MouseLeft);

if (Input::GetAction("Jump")) { }
if (Input::GetActionDown("Shoot")) { }
if (Input::GetActionUp("Reload")) { }

Vec2 mouse = Input::GetMousePos();
Vec2 delta = Input::GetMouseDelta();
Input::SetMouseLocked(true);
```

### Graphics
```cpp
Graphics::Clear(0.1f, 0.1f, 0.15f);
Graphics::SetViewport(0, 0, 1280, 720);
```

### Camera
```cpp
Camera3D::Create();
Camera3D::SetPosition(0, 2, 5);
Camera3D::SetRotation(pitch, yaw);
Camera3D::SetFOV(70.0f);
Camera3D::Move(forward, right, up, deltaTime);
Camera3D::Rotate(mouseDX, mouseDY);
Mat4 view = Camera3D::GetViewMatrix();
```

### Mesh
```cpp
MeshHandle cube = Mesh::CreateCube(1.0f);
MeshHandle sphere = Mesh::CreateSphere(0.5f);
MeshHandle plane = Mesh::CreatePlane(10.0f, 10.0f);
MeshHandle model = Mesh::LoadGLTF("player.glb");
Mesh::Draw(cube, position, rotation, scale);
Mesh::Destroy(cube);
```

### Material
```cpp
MaterialHandle mat = Material::Create();
Material::SetColor(mat, 1.0f, 0.5f, 0.2f, 1.0f);
Material::SetMetallic(mat, 0.8f);
Material::SetRoughness(mat, 0.2f);
Material::SetTexture(mat, textureHandle);
```

### Light
```cpp
Light::AddDirectional(dirX, dirY, dirZ, r, g, b, intensity);
Light::AddPoint(x, y, z, r, g, b, intensity, range);
Light::AddSpot(x, y, z, dirX, dirY, dirZ, r, g, b, intensity, angle);
Light::Clear();
```

### Physics
```cpp
Physics::Init();
Physics::SetGravity(0, -9.81f, 0);
Physics::Step(deltaTime);

ColliderID col = Physics::AddSphereCollider(entityID, radius);
ColliderID col = Physics::AddBoxCollider(entityID, halfX, halfY, halfZ);
ColliderID col = Physics::AddCapsuleCollider(entityID, radius, height);

RayHit hit = Physics::Raycast(originX, originY, originZ, dirX, dirY, dirZ, maxDist);
if (hit.valid) {
    Vec3 point = hit.point;
    Vec3 normal = hit.normal;
    EntityID entity = hit.entity;
}

std::vector<EntityID> hits = Physics::OverlapSphere(x, y, z, radius);
```

### Entity
```cpp
EntityID player = Entity::Create();
Entity::SetPosition(player, x, y, z);
Entity::SetRotation(player, pitch, yaw, roll);
Entity::SetScale(player, sx, sy, sz);
Entity::Destroy(player);

Vec3 pos = Entity::GetPosition(player);
```

### Audio
```cpp
Audio::Init();
SoundID snd = Audio::Load("shot.wav");
Audio::Play(snd);
Audio::PlayAt(snd, x, y, z);
Audio::SetVolume(snd, 0.8f);
Audio::SetListener(camX, camY, camZ, forwardX, forwardY, forwardZ);
```

### Network (Client)
```cpp
NetClient::Connect("127.0.0.1", 27015);
NetClient::Disconnect();
bool connected = NetClient::IsConnected();
NetClient::Send(data, size, reliable);
while (NetClient::HasPacket()) {
    Packet pkt = NetClient::Receive();
}
float ping = NetClient::GetPing();
```

### Network (Server)
```cpp
NetServer::Start(27015, maxPlayers);
NetServer::Stop();
NetServer::Broadcast(data, size, reliable);
NetServer::SendTo(clientID, data, size, reliable);
NetServer::Kick(clientID);
int count = NetServer::GetPlayerCount();
```

### Time
```cpp
float dt = Time::GetDelta();
float total = Time::GetTime();
int fps = Time::GetFPS();
```

### Debug
```cpp
Debug::Log("Player spawned at %f %f %f", x, y, z);
Debug::Warning("Low health!");
Debug::Error("Failed to load model");
Debug::DrawLine(x1, y1, z1, x2, y2, z2, r, g, b);
Debug::DrawSphere(x, y, z, radius, r, g, b);
Debug::DrawBox(x, y, z, sizeX, sizeY, sizeZ, r, g, b);
```

## Example Game

```cpp
#include "Titan.hpp"

int main() {
    Window::Create("My Game", 1280, 720);
    
    Input::MapAction("Forward", KeyCode::W);
    Input::MapAction("Back", KeyCode::S);
    Input::MapAction("Left", KeyCode::A);
    Input::MapAction("Right", KeyCode::D);
    Input::MapAction("Jump", KeyCode::Space);
    Input::MapAction("Shoot", KeyCode::MouseLeft);
    Input::SetMouseLocked(true);
    
    Camera3D::Create();
    Camera3D::SetPosition(0, 2, 5);
    Camera3D::SetFOV(70.0f);
    
    Physics::Init();
    
    MeshHandle floor = Mesh::CreatePlane(50, 50);
    MeshHandle cube = Mesh::CreateCube(1.0f);
    
    Light::AddDirectional(-0.5f, -1.0f, -0.3f, 1.0f, 0.95f, 0.9f, 1.5f);
    
    while (Window::IsOpen()) {
        float dt = Time::GetDelta();
        
        Vec2 mouse = Input::GetMouseDelta();
        Camera3D::Rotate(mouse.x, mouse.y);
        
        float fwd = 0, right = 0;
        if (Input::GetAction("Forward")) fwd += 1;
        if (Input::GetAction("Back")) fwd -= 1;
        if (Input::GetAction("Left")) right -= 1;
        if (Input::GetAction("Right")) right += 1;
        Camera3D::Move(fwd, right, 0, dt);
        
        if (Input::GetActionDown("Shoot")) {
            RayHit hit = Physics::Raycast(
                Camera3D::GetPosition().x,
                Camera3D::GetPosition().y,
                Camera3D::GetPosition().z,
                Camera3D::GetForward().x,
                Camera3D::GetForward().y,
                Camera3D::GetForward().z,
                100.0f
            );
            if (hit.valid) {
                Debug::Log("Hit at %f %f %f", hit.point.x, hit.point.y, hit.point.z);
            }
        }
        
        Graphics::Clear(0.1f, 0.1f, 0.15f);
        Mesh::Draw(floor, {0,0,0}, {0,0,0}, {1,1,1});
        Mesh::Draw(cube, {0,0.5f,0}, {0,0,0}, {1,1,1});
        
        Window::SwapBuffers();
    }
    
    return 0;
}
```

## Multiplayer Example

```cpp
#include "Titan.hpp"

bool isServer = false;

int main(int argc, char** argv) {
    if (argc > 1 && strcmp(argv[1], "-server") == 0) {
        isServer = true;
    }
    
    if (isServer) {
        NetServer::Start(27015, 16);
        Debug::Log("Server started on port 27015");
        
        while (true) {
            NetServer::Update();
            
            while (NetServer::HasEvent()) {
                NetEvent evt = NetServer::GetEvent();
                if (evt.type == NetEventType::Connect) {
                    Debug::Log("Player %d connected", evt.clientID);
                }
                if (evt.type == NetEventType::Disconnect) {
                    Debug::Log("Player %d disconnected", evt.clientID);
                }
                if (evt.type == NetEventType::Data) {
                    NetServer::Broadcast(evt.data, evt.size, true);
                }
            }
            
            Time::Sleep(1);
        }
    } else {
        Window::Create("Game Client", 1280, 720);
        NetClient::Connect("127.0.0.1", 27015);
        
        while (Window::IsOpen()) {
            NetClient::Update();
            
            while (NetClient::HasPacket()) {
                Packet pkt = NetClient::Receive();
            }
            
            Graphics::Clear(0.1f, 0.1f, 0.15f);
            Window::SwapBuffers();
        }
        
        NetClient::Disconnect();
    }
    
    return 0;
}
```


