#pragma once
#include "Components/Component.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

class GameObject final {
public:
    explicit GameObject(std::string name = {});
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;

    const std::string& GetName() const { return name; }
    void SetName(std::string value) { name = std::move(value); }
    GameObject* GetParent() const { return parent; }
    const auto& GetChildren() const { return children; }
    GameObject& AddChild(std::string name);
    GameObject& AdoptChild(std::unique_ptr<GameObject> child);
    bool RemoveChild(GameObject& child);
    void ClearChildren() { children.clear(); }

    void Update(float dt);
    glm::mat4 GetGlobalMatrix();
    bool SetGlobalMatrix(const glm::mat4& matrix);
    bool GetWorldBounds(glm::vec3& low, glm::vec3& high);

    template<class T, class... Args>
    T* AddComponent(Args&&... args) {
        static_assert(std::is_base_of_v<Component, T>);
        if (auto* existing = GetComponent<T>()) return existing;
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        component->owner = this;
        T& result = *component;
        components.push_back(std::move(component));
        return &result;
    }

    template<class T>
    T* GetComponent() {
        for (const auto& component : components)
            if (auto* match = dynamic_cast<T*>(component.get())) return match;
        return nullptr;
    }

    template<class T>
    bool RemoveComponent() {
        const auto found = std::find_if(components.begin(), components.end(),
            [](const auto& item) { return dynamic_cast<T*>(item.get()) != nullptr; });
        if (found == components.end()) return false;
        components.erase(found);
        return true;
    }

private:
    std::string name;
    GameObject* parent = nullptr;
    std::vector<std::unique_ptr<GameObject>> children;
    std::vector<std::unique_ptr<Component>> components;
};
