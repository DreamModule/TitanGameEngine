#pragma once
#include <typeindex>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <functional>

namespace Titan::ECS {

using ComponentTypeId = uint32_t;
constexpr ComponentTypeId INVALID_COMPONENT_TYPE = 0;

struct ComponentTypeInfo {
ComponentTypeId typeId;
std::type_index typeIndex;
std::string name;
size_t size;
size_t alignment;
std::function<void(void*)> constructor;
std::function<void(void*)> destructor;
std::function<void(void*, const void*)> copyAssign;
std::function<void(void*, void*)> moveAssign;
};

class ComponentRegistry {
public:
static ComponentRegistry& Get() {
static ComponentRegistry instance;
return instance;
}

```
template<typename T>
ComponentTypeId Register() {
    std::type_index typeIdx = std::type_index(typeid(T));
    
    auto it = typeIndexToId.find(typeIdx);
    if (it != typeIndexToId.end()) {
        return it->second;
    }
    
    ComponentTypeId typeId = nextTypeId++;
    
    ComponentTypeInfo info;
    info.typeId = typeId;
    info.typeIndex = typeIdx;
    info.name = typeid(T).name();
    info.size = sizeof(T);
    info.alignment = alignof(T);
    
    info.constructor = [](void* ptr) {
        new (ptr) T();
    };
    
    info.destructor = [](void* ptr) {
        static_cast<T*>(ptr)->~T();
    };
    
    info.copyAssign = [](void* dst, const void* src) {
        *static_cast<T*>(dst) = *static_cast<const T*>(src);
    };
    
    info.moveAssign = [](void* dst, void* src) {
        *static_cast<T*>(dst) = std::move(*static_cast<T*>(src));
    };
    
    typeInfos[typeId] = info;
    typeIndexToId[typeIdx] = typeId;
    
    return typeId;
}

template<typename T>
ComponentTypeId GetTypeId() {
    std::type_index typeIdx = std::type_index(typeid(T));
    auto it = typeIndexToId.find(typeIdx);
    if (it != typeIndexToId.end()) {
        return it->second;
    }
    return Register<T>();
}

template<typename T>
bool IsRegistered() const {
    std::type_index typeIdx = std::type_index(typeid(T));
    return typeIndexToId.find(typeIdx) != typeIndexToId.end();
}

const ComponentTypeInfo* GetTypeInfo(ComponentTypeId typeId) const {
    auto it = typeInfos.find(typeId);
    return (it != typeInfos.end()) ? &it->second : nullptr;
}

void Clear() {
    typeInfos.clear();
    typeIndexToId.clear();
    nextTypeId = 1;
}

size_t GetRegisteredCount() const {
    return typeInfos.size();
}
```

private:
ComponentRegistry() : nextTypeId(1) {}
~ComponentRegistry() = default;

```
ComponentRegistry(const ComponentRegistry&) = delete;
ComponentRegistry& operator=(const ComponentRegistry&) = delete;

std::unordered_map<ComponentTypeId, ComponentTypeInfo> typeInfos;
std::unordered_map<std::type_index, ComponentTypeId> typeIndexToId;
ComponentTypeId nextTypeId;
```

};

}
