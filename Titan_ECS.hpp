#ifndef TITAN_ECS_HPP
#define TITAN_ECS_HPP

#include "Titan_Core.hpp"
#include "Titan_Graphics.hpp"
#include "Titan_Assets.hpp"
#include "Titan_Events.hpp"

namespace Titan::ECS {

using EntityID = u32;
const EntityID MAX_ENTITIES = 10000;

struct IPool {
    virtual ~IPool() {}
    virtual void Remove(EntityID e)=0;
};

template<typename T>
struct ComponentPool : IPool {
    std::vector<T> data;
    std::vector<EntityID> sparse;
    std::vector<EntityID> dense;
    std::vector<bool> has;

    ComponentPool() { sparse.resize(MAX_ENTITIES); has.resize(MAX_ENTITIES, false); }
    T& Add(EntityID e, const T& v) {
        if(has[e]) return data[sparse[e]];
        data.push_back(v);
        dense.push_back(e);
        sparse[e]=(u32)data.size()-1;
        has[e]=true;
        return data.back();
    }
    void Remove(EntityID e) override {
        if(!has[e]) return;
        u32 idx = sparse[e];
        u32 last = (u32)data.size()-1;
        EntityID le = dense[last];
        data[idx]=data[last]; dense[idx]=le; sparse[le]=idx;
        data.pop_back(); dense.pop_back(); has[e]=false;
    }
    T* Get(EntityID e) { if(!has[e]) return nullptr; return &data[sparse[e]]; }
};

struct World {
    EntityID counter = 0;
    std::vector<EntityID> entities;
    std::vector<EntityID> pending;
    IPool* pools[32]={nullptr};

    EntityID CreateEntity() {
        EntityID id = ++counter;
        entities.push_back(id);
        return id;
    }
    void DestroyEntity(EntityID id) { pending.push_back(id); }
    template<typename T> void RegisterComponent(u32 id) { pools[id]=new ComponentPool<T>(); }
    template<typename T> T& AddComponent(EntityID e, u32 id, const T& v) { return ((ComponentPool<T>*)pools[id])->Add(e,v); }
    template<typename T> T* GetComponent(EntityID e, u32 id) { return ((ComponentPool<T>*)pools[id])->Get(e); }
    template<typename T> ComponentPool<T>* GetPool(u32 id) { return (ComponentPool<T>*)pools[id]; }
    void Flush() {
        for(EntityID e : pending) for(int i=0;i<32;++i) if(pools[i]) pools[i]->Remove(e);
        pending.clear();
    }
    void Clear() {
        for(int i=0;i<32;++i) if(pools[i]) { delete pools[i]; pools[i]=nullptr; } 
        entities.clear(); counter=0; pending.clear();
    }
};
}

namespace Titan {
    enum ComponentType { 
        COMP_TRANSFORM=0, COMP_SPRITE=1, COMP_CONTROLLER=2, COMP_VELOCITY=3, 
        COMP_LIFETIME=4, COMP_UI_RECT=5, COMP_UI_BUTTON=6, COMP_UI_TEXT=7 
    };

    struct TransformComponent { Math::Vec3 position; Math::Vec3 scale; f32 rotation; };
    struct SpriteComponent { Assets::Material* material; Math::Vec2 size; bool visible; };
    struct PlayerControllerComponent { f32 speed; };
    struct VelocityComponent { Math::Vec3 vel; };
    struct LifetimeComponent { f32 time; };
    
    namespace UI {
        struct RectComponent { Math::Vec2 size; Math::Vec4 color; Math::Vec4 defCol, hovCol, prsCol; i32 zIndex; };
        struct ButtonComponent { bool isHovered; bool isPressed; EventID onClick; };
        struct TextComponent { char text[128]; Assets::Font* font; Math::Vec4 color; bool centered; i32 zIndex; void SetText(const char* s){strncpy(text,s,127);} };
    }
}
#endif
