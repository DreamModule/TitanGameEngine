/**
 * Titan ECS (Entity Component System)
 * Version 2.0.0
 * 
 * Unified ECS implementation with:
 * - Sparse Set for O(1) component access
 * - Entity generations for use-after-free protection
 * - Type-safe API with templates
 * - Each<Components...>() for iteration
 * 
 * Follows Unreal Engine naming conventions
 */

#ifndef TITAN_ECS_HPP
#define TITAN_ECS_HPP

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <queue>
#include <typeindex>
#include <memory>
#include <functional>
#include <algorithm>
#include <cassert>
#include <type_traits>
#include "Titan_Core.hpp"

namespace Titan::ECS {

// ============================================================================
// Entity Definition with Generations
// ============================================================================

using FEntityID = uint32_t;

static constexpr uint32_t ENTITY_INDEX_BITS = 24;
static constexpr uint32_t ENTITY_GENERATION_BITS = 8;
static constexpr uint32_t ENTITY_INDEX_MASK = (1u << ENTITY_INDEX_BITS) - 1u;
static constexpr uint32_t ENTITY_GENERATION_MASK = (1u << ENTITY_GENERATION_BITS) - 1u;
static constexpr FEntityID NULL_ENTITY = 0;
static constexpr uint32_t MAX_ENTITIES = 100000;

/**
 * Extract index from entity ID
 */
inline uint32_t GetEntityIndex(FEntityID Entity)
{
    return Entity & ENTITY_INDEX_MASK;
}

/**
 * Extract generation from entity ID
 */
inline uint32_t GetEntityGeneration(FEntityID Entity)
{
    return (Entity >> ENTITY_INDEX_BITS) & ENTITY_GENERATION_MASK;
}

/**
 * Create entity ID from index and generation
 */
inline FEntityID MakeEntity(uint32_t Index, uint32_t Generation)
{
    return (Generation << ENTITY_INDEX_BITS) | Index;
}

/**
 * Check if entity ID is valid (non-null)
 */
inline bool IsValidEntity(FEntityID Entity)
{
    return Entity != NULL_ENTITY;
}

// ============================================================================
// Component Pool Interface
// ============================================================================

class IComponentPool
{
public:
    virtual ~IComponentPool() = default;
    virtual void OnEntityDestroyed(FEntityID Entity) = 0;
    virtual bool HasComponent(FEntityID Entity) const = 0;
    virtual size_t GetCount() const = 0;
};

// ============================================================================
// Sparse Set Component Pool
// ============================================================================

template<typename T>
class TComponentPool : public IComponentPool
{
public:
    TComponentPool()
    {
        Sparse.resize(MAX_ENTITIES, UINT32_MAX);
    }

    /**
     * Add component to entity
     */
    T& Add(FEntityID Entity, const T& Component)
    {
        uint32_t Index = GetEntityIndex(Entity);
        
        if (Index >= Sparse.size())
        {
            Sparse.resize(Index + 1, UINT32_MAX);
        }

        // If already has component, update and return
        if (Sparse[Index] != UINT32_MAX)
        {
            Dense[Sparse[Index]].Component = Component;
            return Dense[Sparse[Index]].Component;
        }

        // Add new component
        uint32_t DenseIndex = static_cast<uint32_t>(Dense.size());
        Sparse[Index] = DenseIndex;
        Dense.push_back({Entity, Component});
        
        return Dense.back().Component;
    }

    /**
     * Add component with move semantics
     */
    T& Add(FEntityID Entity, T&& Component)
    {
        uint32_t Index = GetEntityIndex(Entity);
        
        if (Index >= Sparse.size())
        {
            Sparse.resize(Index + 1, UINT32_MAX);
        }

        if (Sparse[Index] != UINT32_MAX)
        {
            Dense[Sparse[Index]].Component = std::move(Component);
            return Dense[Sparse[Index]].Component;
        }

        uint32_t DenseIndex = static_cast<uint32_t>(Dense.size());
        Sparse[Index] = DenseIndex;
        Dense.push_back({Entity, std::move(Component)});
        
        return Dense.back().Component;
    }

    /**
     * Remove component from entity
     */
    void Remove(FEntityID Entity)
    {
        uint32_t Index = GetEntityIndex(Entity);
        
        if (Index >= Sparse.size() || Sparse[Index] == UINT32_MAX)
        {
            return;
        }

        uint32_t DenseIndex = Sparse[Index];
        uint32_t LastIndex = static_cast<uint32_t>(Dense.size()) - 1;

        if (DenseIndex != LastIndex)
        {
            // Swap with last element
            Dense[DenseIndex] = std::move(Dense[LastIndex]);
            Sparse[GetEntityIndex(Dense[DenseIndex].Entity)] = DenseIndex;
        }

        Dense.pop_back();
        Sparse[Index] = UINT32_MAX;
    }

    /**
     * Get pointer to component (nullptr if not found)
     */
    T* Get(FEntityID Entity)
    {
        uint32_t Index = GetEntityIndex(Entity);
        
        if (Index >= Sparse.size() || Sparse[Index] == UINT32_MAX)
        {
            return nullptr;
        }

        return &Dense[Sparse[Index]].Component;
    }

    /**
     * Get const pointer to component
     */
    const T* Get(FEntityID Entity) const
    {
        uint32_t Index = GetEntityIndex(Entity);
        
        if (Index >= Sparse.size() || Sparse[Index] == UINT32_MAX)
        {
            return nullptr;
        }

        return &Dense[Sparse[Index]].Component;
    }

    /**
     * Check if entity has this component
     */
    bool HasComponent(FEntityID Entity) const override
    {
        uint32_t Index = GetEntityIndex(Entity);
        return Index < Sparse.size() && Sparse[Index] != UINT32_MAX;
    }

    /**
     * Called when entity is destroyed
     */
    void OnEntityDestroyed(FEntityID Entity) override
    {
        Remove(Entity);
    }

    /**
     * Get number of components
     */
    size_t GetCount() const override
    {
        return Dense.size();
    }

    /**
     * Get all entities with this component
     */
    const auto& GetEntities() const
    {
        return Dense;
    }

    /**
     * Iterator support for range-based for loops
     */
    auto begin() { return Dense.begin(); }
    auto end() { return Dense.end(); }
    auto begin() const { return Dense.begin(); }
    auto end() const { return Dense.end(); }

private:
    struct FDenseElement
    {
        FEntityID Entity;
        T Component;
    };

    std::vector<uint32_t> Sparse;    // Entity index -> Dense index
    std::vector<FDenseElement> Dense; // Packed components
};

// ============================================================================
// System Interface
// ============================================================================

class FWorld; // Forward declaration

class ISystem
{
public:
    virtual ~ISystem() = default;
    virtual void Init(FWorld& World) {}
    virtual void Update(FWorld& World, float DeltaTime) = 0;
    virtual void Shutdown(FWorld& World) {}
    virtual int GetPriority() const { return 0; }
};

// ============================================================================
// World - Main ECS Container
// ============================================================================

class FWorld
{
public:
    FWorld()
        : NextEntityIndex(1)
    {
        Generations.resize(MAX_ENTITIES, 0);
    }

    ~FWorld()
    {
        Clear();
    }

    // ========================================================================
    // Entity Management
    // ========================================================================

    /**
     * Create a new entity
     */
    FEntityID CreateEntity()
    {
        uint32_t Index;
        uint32_t Generation;

        if (!FreeIndices.empty())
        {
            Index = FreeIndices.front();
            FreeIndices.pop();
            Generation = Generations[Index];
        }
        else
        {
            Index = NextEntityIndex++;
            
            if (Index >= Generations.size())
            {
                Generations.resize(Index + 1, 0);
            }
            
            Generation = 0;
        }

        FEntityID Entity = MakeEntity(Index, Generation);
        Entities.push_back(Entity);
        EntityToIndex[Entity] = Entities.size() - 1;
        
        return Entity;
    }

    /**
     * Destroy an entity (deferred until Flush())
     */
    void DestroyEntity(FEntityID Entity)
    {
        if (!IsEntityValid(Entity))
        {
            return;
        }

        PendingDestroy.push_back(Entity);
    }

    /**
     * Check if entity is valid (exists and correct generation)
     */
    bool IsEntityValid(FEntityID Entity) const
    {
        if (Entity == NULL_ENTITY)
        {
            return false;
        }

        uint32_t Index = GetEntityIndex(Entity);
        uint32_t Generation = GetEntityGeneration(Entity);

        if (Index >= Generations.size())
        {
            return false;
        }

        return Generations[Index] == Generation && EntityToIndex.count(Entity) > 0;
    }

    /**
     * Get number of alive entities
     */
    size_t GetEntityCount() const
    {
        return Entities.size();
    }

    /**
     * Get all entities
     */
    const std::vector<FEntityID>& GetEntities() const
    {
        return Entities;
    }

    // ========================================================================
    // Component Management
    // ========================================================================

    /**
     * Register a component type (optional, auto-registered on first use)
     */
    template<typename T>
    void RegisterComponent()
    {
        GetOrCreatePool<T>();
    }

    /**
     * Add component to entity
     */
    template<typename T>
    T& AddComponent(FEntityID Entity, const T& Component)
    {
        assert(IsEntityValid(Entity));
        auto* Pool = GetOrCreatePool<T>();
        return Pool->Add(Entity, Component);
    }

    /**
     * Add component with move semantics
     */
    template<typename T>
    T& AddComponent(FEntityID Entity, T&& Component)
    {
        assert(IsEntityValid(Entity));
        auto* Pool = GetOrCreatePool<T>();
        return Pool->Add(Entity, std::forward<T>(Component));
    }

    /**
     * Add component with constructor arguments
     */
    template<typename T, typename... Args>
    T& EmplaceComponent(FEntityID Entity, Args&&... ConstructorArgs)
    {
        assert(IsEntityValid(Entity));
        auto* Pool = GetOrCreatePool<T>();
        return Pool->Add(Entity, T{std::forward<Args>(ConstructorArgs)...});
    }

    /**
     * Remove component from entity
     */
    template<typename T>
    void RemoveComponent(FEntityID Entity)
    {
        auto* Pool = GetPool<T>();
        if (Pool)
        {
            Pool->Remove(Entity);
        }
    }

    /**
     * Get component (returns nullptr if not found)
     */
    template<typename T>
    T* GetComponent(FEntityID Entity)
    {
        auto* Pool = GetPool<T>();
        if (!Pool)
        {
            return nullptr;
        }
        return Pool->Get(Entity);
    }

    /**
     * Get const component
     */
    template<typename T>
    const T* GetComponent(FEntityID Entity) const
    {
        auto* Pool = GetPool<T>();
        if (!Pool)
        {
            return nullptr;
        }
        return Pool->Get(Entity);
    }

    /**
     * Check if entity has component
     */
    template<typename T>
    bool HasComponent(FEntityID Entity) const
    {
        auto* Pool = GetPool<T>();
        return Pool && Pool->HasComponent(Entity);
    }

    /**
     * Check if entity has all specified components
     */
    template<typename... Components>
    bool HasComponents(FEntityID Entity) const
    {
        return (HasComponent<Components>(Entity) && ...);
    }

    /**
     * Get component pool
     */
    template<typename T>
    TComponentPool<T>* GetPool()
    {
        std::type_index TypeIndex(typeid(T));
        auto It = ComponentPools.find(TypeIndex);
        if (It == ComponentPools.end())
        {
            return nullptr;
        }
        return static_cast<TComponentPool<T>*>(It->second.get());
    }

    /**
     * Get const component pool
     */
    template<typename T>
    const TComponentPool<T>* GetPool() const
    {
        std::type_index TypeIndex(typeid(T));
        auto It = ComponentPools.find(TypeIndex);
        if (It == ComponentPools.end())
        {
            return nullptr;
        }
        return static_cast<const TComponentPool<T>*>(It->second.get());
    }

    // ========================================================================
    // Iteration
    // ========================================================================

    /**
     * Iterate over all entities with specified components
     * Usage: World.Each<Transform, Rigidbody>([](FEntityID E, Transform& T, Rigidbody& R) { ... });
     */
    template<typename... Components, typename Func>
    void Each(Func&& Function)
    {
        // Find the smallest pool to iterate
        auto* FirstPool = GetPool<typename std::tuple_element<0, std::tuple<Components...>>::type>();
        if (!FirstPool)
        {
            return;
        }

        for (auto& Element : *FirstPool)
        {
            FEntityID Entity = Element.Entity;
            
            // Check if entity has all required components
            if ((HasComponent<Components>(Entity) && ...))
            {
                Function(Entity, *GetComponent<Components>(Entity)...);
            }
        }
    }

    /**
     * Iterate with const access
     */
    template<typename... Components, typename Func>
    void Each(Func&& Function) const
    {
        auto* FirstPool = GetPool<typename std::tuple_element<0, std::tuple<Components...>>::type>();
        if (!FirstPool)
        {
            return;
        }

        for (const auto& Element : *FirstPool)
        {
            FEntityID Entity = Element.Entity;
            
            if ((HasComponent<Components>(Entity) && ...))
            {
                Function(Entity, *GetComponent<Components>(Entity)...);
            }
        }
    }

    // ========================================================================
    // World Management
    // ========================================================================

    /**
     * Process pending entity destructions
     */
    void Flush()
    {
        for (FEntityID Entity : PendingDestroy)
        {
            if (!IsEntityValid(Entity))
            {
                continue;
            }

            // Remove all components
            for (auto& [TypeIndex, Pool] : ComponentPools)
            {
                Pool->OnEntityDestroyed(Entity);
            }

            // Remove from entity list
            auto It = EntityToIndex.find(Entity);
            if (It != EntityToIndex.end())
            {
                size_t Index = It->second;
                size_t LastIndex = Entities.size() - 1;

                if (Index != LastIndex)
                {
                    Entities[Index] = Entities[LastIndex];
                    EntityToIndex[Entities[Index]] = Index;
                }

                Entities.pop_back();
                EntityToIndex.erase(Entity);
            }

            // Increment generation and recycle index
            uint32_t EntityIndex = GetEntityIndex(Entity);
            Generations[EntityIndex]++;
            if (Generations[EntityIndex] > ENTITY_GENERATION_MASK)
            {
                Generations[EntityIndex] = 0;
            }
            FreeIndices.push(EntityIndex);
        }

        PendingDestroy.clear();
    }

    /**
     * Clear all entities and components
     */
    void Clear()
    {
        for (auto& [TypeIndex, Pool] : ComponentPools)
        {
            for (FEntityID Entity : Entities)
            {
                Pool->OnEntityDestroyed(Entity);
            }
        }

        Entities.clear();
        EntityToIndex.clear();
        ComponentPools.clear();
        PendingDestroy.clear();

        while (!FreeIndices.empty())
        {
            FreeIndices.pop();
        }

        std::fill(Generations.begin(), Generations.end(), 0);
        NextEntityIndex = 1;
    }

private:
    template<typename T>
    TComponentPool<T>* GetOrCreatePool()
    {
        std::type_index TypeIndex(typeid(T));
        auto It = ComponentPools.find(TypeIndex);

        if (It == ComponentPools.end())
        {
            auto Pool = std::make_unique<TComponentPool<T>>();
            auto* PoolPtr = Pool.get();
            ComponentPools[TypeIndex] = std::move(Pool);
            return PoolPtr;
        }

        return static_cast<TComponentPool<T>*>(It->second.get());
    }

    // Entity storage
    std::vector<FEntityID> Entities;
    std::unordered_map<FEntityID, size_t> EntityToIndex;
    std::vector<uint32_t> Generations;
    std::queue<uint32_t> FreeIndices;
    uint32_t NextEntityIndex;
    std::vector<FEntityID> PendingDestroy;

    // Component storage
    std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> ComponentPools;
};

// ============================================================================
// Scheduler
// ============================================================================

class FScheduler
{
public:
    ~FScheduler()
    {
        Clear();
    }

    /**
     * Register a system
     */
    void Register(ISystem* System)
    {
        Systems.push_back(std::unique_ptr<ISystem>(System));
        std::sort(Systems.begin(), Systems.end(), 
            [](const auto& A, const auto& B) 
            {
                return A->GetPriority() < B->GetPriority();
            });
    }

    /**
     * Initialize all systems
     */
    void Init(FWorld& World)
    {
        for (auto& System : Systems)
        {
            System->Init(World);
        }
    }

    /**
     * Update all systems
     */
    void Update(FWorld& World, float DeltaTime)
    {
        for (auto& System : Systems)
        {
            System->Update(World, DeltaTime);
        }
        World.Flush();
    }

    /**
     * Shutdown all systems (in reverse order)
     */
    void Shutdown(FWorld& World)
    {
        for (auto It = Systems.rbegin(); It != Systems.rend(); ++It)
        {
            (*It)->Shutdown(World);
        }
        Clear();
    }

    /**
     * Clear all systems
     */
    void Clear()
    {
        Systems.clear();
    }

private:
    std::vector<std::unique_ptr<ISystem>> Systems;
};

// ============================================================================
// Built-in Component Types
// ============================================================================

enum class EComponentType : uint32_t
{
    Transform = 0,
    Rigidbody = 1,
    Collider = 2,
    UIRect = 3,
    UIText = 4,
    UIJoystick = 5
};

struct FTransformComponent
{
    Math::Vec3 Position{0, 0, 0};
    Math::Vec3 Scale{1, 1, 1};
    float Rotation = 0.0f;
};

struct FRigidbodyComponent
{
    Math::Vec3 Velocity{0, 0, 0};
    Math::Vec3 AngularVelocity{0, 0, 0};
    float Mass = 1.0f;
    float Drag = 0.0f;
    float AngularDrag = 0.05f;
    bool UseGravity = true;
    bool IsKinematic = false;
};

struct FColliderComponent
{
    Math::Vec3 Center{0, 0, 0};
    Math::Vec3 Size{1, 1, 1};
    float Radius = 0.5f;
    bool IsTrigger = false;
};

struct FUIRectComponent
{
    Math::Vec2 Size{100, 100};
    Math::Vec4 Color{1, 1, 1, 1};
    int32_t ZIndex = 0;
};

struct FUITextComponent
{
    char Text[256] = "";
    Math::Vec4 Color{1, 1, 1, 1};
    float FontSize = 16.0f;
    bool Centered = false;
};

// ============================================================================
// Backward Compatibility Aliases
// ============================================================================

using EntityID = FEntityID;
using World = FWorld;
using TransformComponent = FTransformComponent;

// Old-style component IDs for compatibility
static constexpr uint32_t COMP_TRANSFORM = 0;
static constexpr uint32_t COMP_RIGIDBODY = 1;
static constexpr uint32_t COMP_COLLIDER = 2;
static constexpr uint32_t COMP_UI_RECT = 3;
static constexpr uint32_t COMP_UI_TEXT = 4;
static constexpr uint32_t COMP_UI_JOYSTICK = 5;

} // namespace Titan::ECS

#endif // TITAN_ECS_HPP
