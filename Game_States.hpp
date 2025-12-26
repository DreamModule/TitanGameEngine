#ifndef GAME_STATES_HPP
#define GAME_STATES_HPP
#include "Titan_State.hpp"
#include "Game_Features.hpp"
#include "Game_Definitions.hpp"

struct MatchState : Titan::IState {
    const char* GetName() const override { return "Match"; }
    void OnEnter() override {
        Titan::EventBus::Subscribe(Game::Events::EXIT_GAME, [](const Titan::EventContext&){ Titan::Engine::Get()->sm.SwitchState(new MenuState()); });
        Button::GameButton::Create(Titan::Engine::Get()->w, {"EXIT", 10,10,100,40,Game::Events::EXIT_GAME,{0.5,0.5,0.5,1}});
    }
    void OnUpdate(f32 dt) override {
        Titan::Input::Manager::Update();
        Titan::UI::InteractionSystem::Update(Titan::Engine::Get()->w);
        Titan::Graphics::Backend::Clear(0.1f,0.2f,0.1f);
        Titan::UI::RenderSystem::Update(Titan::Engine::Get()->w);
    }
};

struct MenuState : Titan::IState {
    const char* GetName() const override { return "Menu"; }
    void OnEnter() override {
        Titan::EventBus::Subscribe(Game::Events::START_MATCH, [](const Titan::EventContext&){ Titan::Engine::Get()->sm.SwitchState(new MatchState()); });
        Titan::EventBus::Subscribe(Game::Events::EXIT_GAME, [](const Titan::EventContext&){ Titan::Engine::Quit(); });
        Button::GameButton::Create(Titan::Engine::Get()->w, {"PLAY", 540,300,200,60,Game::Events::START_MATCH,{0.2f,0.8f,0.2f,1}});
        Button::GameButton::Create(Titan::Engine::Get()->w, {"QUIT", 540,400,200,60,Game::Events::EXIT_GAME,{0.8f,0.2f,0.2f,1}});
    }
    void OnUpdate(f32 dt) override {
        Titan::Input::Manager::Update();
        Titan::UI::InteractionSystem::Update(Titan::Engine::Get()->w);
        Titan::Graphics::Backend::Clear(0.1f,0.1f,0.2f);
        Titan::UI::RenderSystem::Update(Titan::Engine::Get()->w);
    }
};
#endif
