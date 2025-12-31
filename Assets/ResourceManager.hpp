#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <typeindex>

namespace Titan::Assets {

class ResourceManager {
public:
    static ResourceManager& Get() {
        static ResourceManager instance;
        return instance;
    }
    
    template<typename T>
    std::shared_ptr<T> Load(const std::string& name, const std::string& path) {
        std::type_index type = typeid(T);
        std::string key = std::string(type.name()) + "::" + name;
        
        auto it = resources.find(key);
        if (it != resources.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        
        auto resource = std::make_shared<T>();
        if (!resource->LoadFromFile(path)) {
            return nullptr;
        }
        
        resources[key] = resource;
        return resource;
    }
    
    template<typename T>
    std::shared_ptr<T> Get(const std::string& name) {
        std::type_index type = typeid(T);
        std::string key = std::string(type.name()) + "::" + name;
        
        auto it = resources.find(key);
        if (it != resources.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        return nullptr;
    }
    
    void Clear() {
        resources.clear();
    }

private:
    ResourceManager() = default;
    std::unordered_map<std::string, std::shared_ptr<void>> resources;
};

}
