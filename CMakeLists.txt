cmake_minimum_required(VERSION 3.16)
project(TitanEngine)

set(CMAKE_CXX_STANDARD 17)

add_executable(TitanEngine
    src/main.cpp
    src/Core/Engine.cpp
    src/Platform/Window_Win32.cpp
    src/Graphics/GraphicsDevice_GL.cpp
)

target_include_directories(TitanEngine PRIVATE src)

if (WIN32)
    target_link_libraries(TitanEngine PRIVATE opengl32 user32 gdi32)
endif()
