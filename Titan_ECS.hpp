#ifndef TITAN_ECS_HPP
#define TITAN_ECS_HPP

#include <vector>
#include <unordered_map>
#include "Titan_Core.hpp"

namespace Titan::ECS {
    using EntityID = u32;
    const u32 MAX_ENTITIES_PER_POOL = 10000;

    struct IPool { virtual ~IPool() {} virtual void Remove(EntityID e) = 0; };

    template<typename T> struct ComponentPool : IPool {
        std::vector<T> data;
        std::vector<EntityID> dense;
        std::vector<u32> sparse;
        std::vector<bool> has;

        ComponentPool() { sparse.resize(MAX_ENTITIES_PER_POOL); has.resize(MAX_ENTITIES_PER_POOL, false); }
        T& Add(EntityID e, const T& v) {
            if(has[e]) return data[sparse[e]];
            data.push_back(v); dense.push_back(e); sparse[e]=(u32)data.size()-1; has[e]=true;
            return data.back();
        }
        void Remove(EntityID e) override {
            if(!has[e]) return;
            u32 idx = sparse[e];
            u32 lastIdx = (u32)data.size()-1;
            EntityID lastEntity = dense[lastIdx];
            data[idx] = data[lastIdx]; dense[idx] = lastEntity; sparse[lastEntity] = idx;
            data.pop_back(); dense.pop_back(); has[e] = false;
        }
        T* Get(EntityID e) { return has[e] ? &data[sparse[e]] : nullptr; }
    };

    struct World {
        EntityID counter = 0;
        std::vector<EntityID> entities;
        std::unordered_map<EntityID, size_t> entityToIndex;
        std::vector<EntityID> pending;
        std::vector<IPool*> pools;

        EntityID CreateEntity() {
            EntityID id = ++counter;
            entityToIndex[id] = entities.size();
            entities.push_back(id);
            return id;
        }
        void DestroyEntity(EntityID id) { pending.push_back(id); }
        
        template<typename T> void RegisterComponent(u32 id) { 
            if(id >= pools.size()) pools.resize(id+1, nullptr);
            if(!pools[id]) pools[id]=new ComponentPool<T>(); 
        }
        template<typename T> T& AddComponent(EntityID e, u32 id, const T& v) { return ((ComponentPool<T>*)pools[id])->Add(e,v); }
        template<typename T> T* GetComponent(EntityID e, u32 id) { return ((ComponentPool<T>*)pools[id])->Get(e); }
        template<typename T> ComponentPool<T>* GetPool(u32 id) { 
            if(id >= pools.size()) return nullptr;
            return (ComponentPool<T>*)pools[id]; 
        }

        void Flush() {
            if(pending.empty()) return;
            for(EntityID e : pending) {
                for(auto* p : pools) if(p) p->Remove(e);
                if(entityToIndex.count(e)) {
                    size_t idx = entityToIndex[e];
                    size_t lastIdx = entities.size()-1;
                    EntityID lastE = entities[lastIdx];
                    if(idx != lastIdx) { entities[idx] = lastE; entityToIndex[lastE] = idx; }
                    entities.pop_back(); entityToIndex.erase(e);
                }
            }
            pending.clear();
        }
        void Clear() {
            for(auto* p : pools) if(p) delete p;
            pools.clear(); entities.clear(); entityToIndex.clear(); counter=0; pending.clear();
        }
    };
    
    enum ComponentType { 
        COMP_TRANSFORM=0, COMP_RIGIDBODY=1, COMP_COLLIDER=2, 
        COMP_UI_RECT=3, COMP_UI_TEXT=4, COMP_UI_JOYSTICK=5 
    };
    struct TransformComponent { Math::Vec3 position; Math::Vec3 scale; f32 rotation; };
}
#endif
