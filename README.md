# TitanGameEngine

A lightweight, high-performance 2D/3D game engine written in C++17.

## Features

- **Entity Component System (ECS)** - Sparse set based ECS with O(1) component access
- **OpenGL Renderer** - Modern OpenGL rendering with batching and frustum culling
- **Input System** - Keyboard, mouse, and gamepad support
- **Audio System** - Powered by miniaudio
- **Debug UI** - Immediate mode debug interface
- **TitanShield** - Anti-cheat and memory protection system

## Requirements

- C++17 compatible compiler (MSVC 2019+, GCC 8+, Clang 7+)
- CMake 3.16+
- OpenGL 3.3+
- Windows 10+ (other platforms coming soon)

## Quick Start

### Building

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Basic Usage

```cpp
#include "Titan_Engine.hpp"
#include "Titan_ECS.hpp"

int main()
{
    // Initialize engine
    Titan::Engine::Init("My Game", 1280, 720);

    // Get world reference
    auto& World = Titan::Engine::Get()->world;

    // Create an entity
    auto Player = World.CreateEntity();
    World.AddComponent<Titan::ECS::FTransformComponent>(Player, 
        Titan::Math::Vec3{640, 360, 0},
        Titan::Math::Vec3{1, 1, 1},
        0.0f
    );

    // Game loop
    while (Titan::Engine::IsRunning())
    {
        Titan::Input::Manager::Update();
        
        float DeltaTime = Titan::Platform::GetDeltaTime();
        Titan::Engine::Get()->scheduler.Update(World, DeltaTime);
        
        Titan::Platform::SwapBuffers();
    }

    Titan::Engine::Shutdown();
    return 0;
}
```

## ECS System

The engine uses a custom ECS implementation with:

- **Sparse Set** storage for O(1) component access
- **Entity Generations** for safe entity references
- **Type-safe** component API

### Creating Entities and Components

```cpp
using namespace Titan::ECS;

FWorld World;

// Create entity
FEntityID Entity = World.CreateEntity();

// Add components
World.AddComponent<FTransformComponent>(Entity, 
    Math::Vec3{0, 0, 0},    // Position
    Math::Vec3{1, 1, 1},    // Scale
    0.0f                     // Rotation
);

// Get component
auto* Transform = World.GetComponent<FTransformComponent>(Entity);
if (Transform)
{
    Transform->Position.x += 10.0f;
}

// Check if has component
if (World.HasComponent<FTransformComponent>(Entity))
{
    // ...
}

// Remove component
World.RemoveComponent<FTransformComponent>(Entity);

// Destroy entity
World.DestroyEntity(Entity);
World.Flush(); // Process pending destructions
```

### Iterating Over Entities

```cpp
// Iterate all entities with Transform and Rigidbody
World.Each<FTransformComponent, FRigidbodyComponent>(
    [](FEntityID Entity, FTransformComponent& Transform, FRigidbodyComponent& Rigidbody)
    {
        Transform.Position += Rigidbody.Velocity * DeltaTime;
    }
);
```

### Creating Systems

```cpp
struct FPhysicsSystem : public Titan::ECS::ISystem
{
    void Init(FWorld& World) override
    {
        // Initialize physics
    }

    void Update(FWorld& World, float DeltaTime) override
    {
        World.Each<FTransformComponent, FRigidbodyComponent>(
            [DeltaTime](FEntityID E, auto& T, auto& R)
            {
                if (R.UseGravity)
                {
                    R.Velocity.y += -9.81f * DeltaTime;
                }
                T.Position += R.Velocity * DeltaTime;
            }
        );
    }

    int GetPriority() const override { return 100; }
};

// Register system
Engine::Get()->scheduler.Register(new FPhysicsSystem());
```

## TitanShield Protection

Built-in anti-cheat and memory protection:

```cpp
#include "src/Shield/TitanShieldCore.h"

// Initialize protection
TitanShield::Initialize(TitanShield::ProtectionLevel::Enhanced);

// Secure variables
TITAN_SECURE_INT(PlayerHealth, 100);
TITAN_SECURE_FLOAT(PlayerMoney, 1000.0f);

// Access protected values
int Health = PlayerHealth.Get();
PlayerHealth.Set(Health - 10);

// Detect cheats
if (TitanShield::DetectCheatEngine())
{
    // Handle detection
}

// Shutdown
TitanShield::Shutdown();
```

## Project Structure

```
TitanGameEngine/
├── CMakeLists.txt          # Build configuration
├── .clang-format           # Code style settings
├── main.cpp                # Demo application
├── Titan_*.hpp/cpp         # Public API headers
└── src/
    ├── Core/               # Engine core
    ├── ECS/                # Entity Component System
    ├── Graphics/           # Rendering
    ├── Physics/            # Physics system
    ├── Input/              # Input handling
    ├── Platform/           # Platform abstraction
    ├── Scene/              # Scene management
    ├── Assets/             # Asset loading
    ├── Debug/              # Debug tools
    ├── Network/            # Networking
    └── Shield/             # Protection system
```

## Code Style

The project follows Unreal Engine naming conventions:

- **Classes/Structs**: `FClassName`, `UClassName`
- **Enums**: `EEnumName`
- **Interfaces**: `IInterfaceName`
- **Variables**: `PascalCase`
- **Booleans**: `bIsEnabled`

Run clang-format to apply style:

```bash
clang-format -i src/**/*.cpp src/**/*.hpp *.hpp *.cpp
```

## Dependencies

- **miniaudio** - Audio playback (header-only)
- **stb_truetype** - Font rendering (header-only)
- **stb_image** - Image loading (header-only)

## License

See [LICENSE](LICENSE) file.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Apply code style (`clang-format`)
4. Submit a pull request

## Version History

- **v2.0.0** - Major refactor: unified ECS, Unreal-style naming, TitanShield v4
- **v1.3.0** - Added TitanShield protection system
- **v1.2.0** - Added physics and joystick support
- **v1.0.0** - Initial release
(readme generated by opus 4.5)
