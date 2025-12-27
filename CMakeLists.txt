cmake_minimum_required(VERSION 3.16)
project(TitanEngine)

set(CMAKE_CXX_STANDARD 17)

add_executable(TitanEngine
    src/main.cpp
    src/Core/Engine.cpp
    src/Core/Scheduler.cpp
    src/Core/FrameContext.hpp
    src/Core/ISystem.hpp
    src/Core/Scheduler.hpp
    src/Core/Systems/PlatformPollSystem.cpp
    src/Core/Systems/PlatformPollSystem.hpp
    src/Core/Systems/InputSystem.cpp
    src/Core/Systems/InputSystem.hpp
    src/Core/Systems/RenderSystem.cpp
    src/Core/Systems/RenderSystem.hpp
    src/Core/Systems/JoystickSystem.cpp
    src/Core/Systems/JoystickSystem.hpp
    src/Platform/Window_Win32.cpp
    src/Platform/Window.hpp
    src/Input/InputManager.cpp
    src/Input/InputManager.hpp
    src/Input/Joystick.cpp
    src/Input/Joystick.hpp
    src/Graphics/GraphicsDevice_GL.cpp
    src/Graphics/GraphicsDevice.hpp
    src/Assets/Texture.hpp
    src/Assets/Loader.hpp
    src/Assets/Loader.cpp
    src/Debug/Debug.hpp
    src/Debug/Debug.cpp
    src/ECS/Entity.hpp
    src/ECS/World.hpp
    src/ECS/World.cpp
    src/ECS/Components/Transform.hpp
    src/ECS/Components/Sprite.hpp
    src/Scene/SceneManager.hpp
    src/Scene/SceneManager.cpp
    src/Scene/SceneSerializer.hpp
    src/Scene/SceneSerializer.cpp
    src/Core/Engine.hpp
)

target_include_directories(TitanEngine PRIVATE src)

if (WIN32)
    target_link_libraries(TitanEngine PRIVATE opengl32 user32 gdi32 xinput)
endif()
