TitanEngine/
├── Core/
│   ├── Engine.hpp                    (ЗАМЕНИТЬ - новый EngineContext.hpp)
│   ├── Engine.cpp                    (УДАЛИТЬ - логика теперь в Engine.hpp)
│   ├── EngineContext.hpp             (НОВЫЙ - из artifact "engine_context")
│   ├── Time.hpp                      (оставить как есть)
│   ├── Time.cpp                      (оставить как есть)
│   ├── Logger.hpp                    (оставить как есть)
│   ├── Logger.cpp                    (оставить как есть)
│   ├── Scheduler.hpp                 (УДАЛИТЬ - заменён Scene системой)
│   ├── Scheduler.cpp                 (УДАЛИТЬ)
│   ├── ISystem.hpp                   (УДАЛИТЬ - заменён Scene::ISystem)
│   └── FrameContext.hpp              (УДАЛИТЬ - не нужен)
│
├── ECS/
│   ├── Entity.hpp                    (оставить как есть)
│   ├── World.hpp                     (ЗАМЕНИТЬ - из artifact "ecs_world_unified")
│   ├── World.cpp                     (УДАЛИТЬ - всё в .hpp теперь)
│   ├── ComponentManager.hpp          (УДАЛИТЬ - интегрирован в World)
│   ├── System.hpp                    (УДАЛИТЬ - заменён Scene::ISystem)
│   └── Components/
│       ├── Transform.hpp             (оставить - уже 3D версия)
│       ├── Sprite.hpp                (оставить для совместимости)
│       ├── Camera.hpp                (оставить как есть)
│       └── MeshRenderer.hpp          (оставить как есть)
│
├── Scene/
│   ├── SceneManager.hpp              (ЗАМЕНИТЬ - из artifact "scene_manager_unified")
│   ├── SceneManager.cpp              (УДАЛИТЬ - всё в .hpp)
│   ├── SceneSerializer.hpp           (оставить пока)
│   └── SceneSerializer.cpp           (оставить пока)
│
├── Rendering/
│   └── RenderSystem.hpp              (НОВЫЙ - из artifact "render_system_modern")
│
├── Assets/
│   ├── ResourceManager.hpp           (НОВЫЙ - создай этот файл отдельно)
│   ├── Loader.hpp                    (оставить для совместимости)
│   ├── Loader.cpp                    (оставить для совместимости)
│   └── Texture.hpp/cpp               (оставить как есть)
│
├── Graphics/
│   ├── GraphicsDevice.hpp            (оставить как есть)
│   ├── GraphicsDevice_GL.cpp         (оставить как есть)
│   ├── Texture.hpp/cpp               (оставить как есть)
│   ├── Shader.hpp/cpp                (оставить как есть)
│   └── Mesh.hpp                      (оставить как есть)
│
├── Platform/
│   ├── Window.hpp                    (оставить как есть)
│   ├── Window_Win32.cpp              (оставить как есть)
│   ├── Input.hpp                     (оставить как есть)
│   └── Input.cpp                     (оставить как есть)
│
├── Input/
│   ├── InputManager.hpp              (оставить как есть)
│   ├── InputManager.cpp              (оставить как есть)
│   ├── Joystick.hpp                  (оставить как есть)
│   └── Joystick.cpp                  (оставить как есть)
│
├── Systems/
│   ├── RenderSystem.hpp              (УДАЛИТЬ - старая версия)
│   ├── RenderSystem.cpp              (УДАЛИТЬ)
│   ├── RenderEntitySystem.hpp        (УДАЛИТЬ - заменён новым RenderSystem)
│   ├── RenderEntitySystem.cpp        (УДАЛИТЬ)
│   ├── PlatformPollSystem.hpp        (УДАЛИТЬ - интегрировано в Engine)
│   ├── PlatformPollSystem.cpp        (УДАЛИТЬ)
│   ├── InputSystem.hpp               (УДАЛИТЬ - интегрировано в Engine)
│   └── InputSystem.cpp               (УДАЛИТЬ)
│
└── main.cpp                          (ЗАМЕНИТЬ - из artifact "main_example")
