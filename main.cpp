// shit game test
#define _CRT_SECURE_NO_WARNINGS
#include "Titan_Impl.cpp"
#include "Titan_UI.hpp"
#include "Titan_Debug.hpp"
#include <unordered_map>
#include <string>

struct PlayerTag {};

struct WorldPosComponent {
    Titan::f32 x, y;
};

enum class EntityType { Tree, Stone, Flower };

struct EnvironmentComponent {
    Titan::i32 gridX, gridY;
};

struct ForestSystem : Titan::ECS::ISystem {
    
    const int CELL_SIZE = 40;
    const int VIEW_DIST_X = 1400;
    const int VIEW_DIST_Y = 900;
    const float PLAYER_SPEED = 300.0f;

    Titan::Math::Vec2 playerPos = {0, 0};
    bool showDebug = true;
    int renderedEntities = 0;
    
    std::unordered_map<long long, Titan::ECS::EntityID> activeChunks;

    void Init(Titan::ECS::World& w) override {
        using namespace Titan;
        
        ECS::EntityID player = w.CreateEntity();
        w.AddComponent(player, COMP_TRANSFORM, {{640, 360, 0}, {1,1,1}, 0});
        w.AddComponent(player, COMP_UI_RECT, {{20, 30}, {1.0f, 0.2f, 0.2f, 1}});
        w.AddComponent(player, 10, PlayerTag{}); 
        w.AddComponent(player, 11, WorldPosComponent{0, 0}); 

        Input::Manager::MapAction("Up", KeyCode::W);
        Input::Manager::MapAction("Left", KeyCode::A);
        Input::Manager::MapAction("Down", KeyCode::S);
        Input::Manager::MapAction("Right", KeyCode::D);
        Input::Manager::MapAction("ToggleDebug", KeyCode::Space);
    }

    int GetCellType(int x, int y) {
        unsigned int h = (x * 374761393) ^ (y * 668265263);
        h = (h ^ (h >> 13)) * 1274126177;
        int val = h % 100;

        if (val < 5) return 1;
        if (val < 7) return 2;
        if (val < 15) return 3;
        return 0;
    }

    void Update(Titan::ECS::World& w, Titan::f32 dt) override {
        using namespace Titan;

        auto* players = w.GetPool<PlayerTag>(10);
        auto* worldPosPool = w.GetPool<WorldPosComponent>(11);
        
        if (players && !players->data.empty()) {
            ECS::EntityID pid = players->dense[0];
            WorldPosComponent* wPos = worldPosPool->Get(pid);

            if (wPos) {
                Titan::Math::Vec2 move = {0,0};
                if (Input::Manager::GetAction("Up")) move.y -= 1;
                if (Input::Manager::GetAction("Down")) move.y += 1;
                if (Input::Manager::GetAction("Left")) move.x -= 1;
                if (Input::Manager::GetAction("Right")) move.x += 1;

                if (move.x != 0 || move.y != 0) {
                    float len = sqrt(move.x*move.x + move.y*move.y);
                    move.x /= len; move.y /= len;
                    wPos->x += move.x * PLAYER_SPEED * dt;
                    wPos->y += move.y * PLAYER_SPEED * dt;
                }
                playerPos = {wPos->x, wPos->y};
            }
        }

        int startX = (int)(playerPos.x - VIEW_DIST_X/2) / CELL_SIZE - 1;
        int endX   = (int)(playerPos.x + VIEW_DIST_X/2) / CELL_SIZE + 1;
        int startY = (int)(playerPos.y - VIEW_DIST_Y/2) / CELL_SIZE - 1;
        int endY   = (int)(playerPos.y + VIEW_DIST_Y/2) / CELL_SIZE + 1;

        std::vector<long long> toRemove;
        for (auto& kv : activeChunks) {
            EnvironmentComponent* env = w.GetComponent<EnvironmentComponent>(kv.second, 12);
            if (env) {
                if (env->gridX < startX || env->gridX > endX || 
                    env->gridY < startY || env->gridY > endY) {
                    w.DestroyEntity(kv.second);
                    toRemove.push_back(kv.first);
                }
            } else {
                toRemove.push_back(kv.first); 
            }
        }
        for (long long key : toRemove) activeChunks.erase(key);

        for (int y = startY; y <= endY; ++y) {
            for (int x = startX; x <= endX; ++x) {
                long long key = (long long)y * 1000000 + x;
                
                if (activeChunks.find(key) == activeChunks.end()) {
                    int type = GetCellType(x, y);
                    if (type != 0) {
                        ECS::EntityID e = w.CreateEntity();
                        
                        Math::Vec4 color;
                        Math::Vec2 size;
                        
                        if (type == 1) { color = {0.1f, 0.6f, 0.1f, 1}; size = {30, 30}; }
                        else if (type == 2) { color = {0.5f, 0.5f, 0.5f, 1}; size = {20, 20}; }
                        else { color = {1.0f, 1.0f, 0.0f, 1}; size = {10, 10}; }

                        w.AddComponent(e, COMP_UI_RECT, {size, color});
                        w.AddComponent(e, COMP_TRANSFORM, {{0,0,0}, {1,1,1}, 0});
                        w.AddComponent(e, 11, WorldPosComponent{(float)x * CELL_SIZE, (float)y * CELL_SIZE});
                        w.AddComponent(e, 12, EnvironmentComponent{x, y});
                        
                        activeChunks[key] = e;
                    }
                }
            }
        }

        renderedEntities = (int)activeChunks.size();

        auto* transPool = w.GetPool<TransformComponent>(COMP_TRANSFORM);
        auto* wPosPool = w.GetPool<WorldPosComponent>(11);

        float centerX = 1280.0f / 2.0f;
        float centerY = 720.0f / 2.0f;

        if (transPool && wPosPool) {
            for (size_t i = 0; i < wPosPool->data.size(); ++i) {
                ECS::EntityID id = wPosPool->dense[i];
                WorldPosComponent& wp = wPosPool->data[i];
                TransformComponent* t = transPool->Get(id);

                if (t) {
                    t->position.x = wp.x - playerPos.x + centerX;
                    t->position.y = wp.y - playerPos.y + centerY;
                }
            }
        }

        if (Input::Manager::GetActionDown("ToggleDebug")) showDebug = !showDebug;

        if (showDebug) {
            Debug::Begin();
            Debug::Text("TITAN ENGINE v1.3 - FOREST DEMO");
            Debug::Text("FPS: %.1f", 1.0f / (dt > 0.0001f ? dt : 0.016f));
            Debug::Text("Entities (Optimized): %d", renderedEntities);
            Debug::Text("Player Pos: %.1f, %.1f", playerPos.x, playerPos.y);
            
            if (Debug::Button("Exit Game")) {
                Engine::Quit();
            }
            if (Debug::Button("Save Pos")) {
                Titan::Core::SceneSerializer::Save(w, "quicksave.bin");
            }
            Debug::End();
        }
    }

    int GetPriority() const override { return 50; }
};

int main() {
    Titan::Engine::Init("Titan Forest Runner", 1280, 720);

    auto& w = Titan::Engine::Get()->world;
    w.RegisterComponent<PlayerTag>(10);
    w.RegisterComponent<WorldPosComponent>(11);
    w.RegisterComponent<EnvironmentComponent>(12);

    Titan::Engine::Get()->scheduler.Register(new ForestSystem());
    
    Titan::Engine::Get()->scheduler.Register(new Titan::UI::DebugRenderSystem());
    Titan::Engine::Get()->scheduler.Register(new Titan::UI::RenderSystem());

    while(Titan::Engine::IsRunning()) {
        Titan::Input::Manager::Update();
        
        float dt = Titan::Platform::GetTime();
        static float lastTime = 0;
        float realDt = dt - lastTime;
        lastTime = dt;
        if(realDt > 0.1f) realDt = 0.1f;

        Titan::Engine::Get()->scheduler.Update(w, realDt);
        Titan::Platform::SwapBuffers();
    }

    Titan::Engine::Shutdown();
    return 0;
}
