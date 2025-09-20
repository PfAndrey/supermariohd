#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <string>
#include <unordered_map>

#include "Logger.hpp"


/*
 * @class ResourceManager
 * @brief Resource manager class
 * @tparam T - resource type
 */
template <typename T>
class ResourceManager {
public:
    ResourceManager() = default;
    ~ResourceManager() = default;

    // Deleted copy constructor and assignment operator
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager operator=(ResourceManager&&) = delete;
    
    /*
     * @brief Load resource from file
     * @param name [in] - resource name
     * @param file_path [in] - path to file
     * @note Resource type must have loadFromFile method.
     *       Also it's possible to specialize this method for different resource types.
     * @return true if resource was loaded successfully, false otherwise
     */
    bool loadFromFile(const std::string& name, const std::string& file_path) {
        auto it = m_resources.find(name);
        if (it != m_resources.end()) {
            LOG("RES_MNG", ERROR, "Resource with name %s already exists", name.c_str());
            return false;
        }
 
        auto resource = new T();
        if (!resource->loadFromFile(file_path)) {
            LOG("RES_MNG", ERROR, "Failed to load %s", name.c_str());
            return false;
        }

        LOG("RES_MNG", DEBUG, "Loaded resource '%s' from file %s", name.c_str(), file_path.c_str());
        m_resources[name] = resource;
        return true;
    }

    /*
     * @brief Get resource by name
     * @param name [in] - resource name
     * @return pointer to resource if it was found, nullptr otherwise
     */
    T* get(const std::string& name) {
        auto it = m_resources.find(name);
        if (it == m_resources.end()) {
            LOG("RES_MNG", ERROR, "Loaded resource '%s' not found!", name.c_str());
            return nullptr;
        }

        return it->second;
    }

    /*
     * @brief Get resource by name (const version)
     * @param name [in] - resource name
     * @return pointer to resource if it was found, nullptr otherwise
     */
    const T* get(const std::string& name) const {
        auto it = m_resources.find(name);
        if (it == m_resources.end()) {
            LOG("RES_MNG", WARNING, "Resource with name %s not found", name.c_str());
            return nullptr;
        }
        return it->second;
    }

protected:
    std::unordered_map<std::string, T*> m_resources;
};

#endif // !RESOURCE_MANAGER_HPP
