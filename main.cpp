//shit 
#include "Titan_Impl.cpp"
#include "Game_States.hpp"

int main() {
    Titan::Engine::Init("Titan v1.0", 1280, 720);
    Titan::Input::Manager::MapAction("Click", Titan::KeyCode::MouseLeft);
    Titan::Engine::Get()->sm.SwitchState(new MenuState());
    while(Titan::Window::System::Update() && Titan::Engine::IsRunning()) {
        Titan::Engine::Get()->sm.Update(0.016f);
        Titan::Window::System::SwapBuffers();
    }
    Titan::Engine::Shutdown();
    return 0;
}
