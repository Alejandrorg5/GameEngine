#pragma once
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include <glm/glm.hpp>

#include "Components/Component.h"
#include "Components/TransformComponent.h"
#include "Components/MeshComponent.h"


using namespace std;

class GameObject;

class GameObject {

public:
    GameObject();

    GameObject(string n);

    // Destructor
    virtual ~GameObject();

    bool active = true;
    string name;

    GameObject* parent = nullptr;
    vector<unique_ptr<GameObject>> children;
    vector<unique_ptr<Component>> components;

    void Update(float dt);
    glm::mat4 GetGlobalMatrix();
    bool SetGlobalMatrix(const glm::mat4& matrix);
    bool GetWorldBounds(glm::vec3& low, glm::vec3& high);

    template <typename... Args>
    GameObject* AddChild(Args&&... args) {
        // Creamos el nuevo GameObject (puedes pasarle nombre o los argumentos que necesites)
        auto newChild = std::make_unique<GameObject>(std::forward<Args>(args)...);
        GameObject* rawPtr = newChild.get();

        // Asignamos este objeto como su padre
        newChild->parent = this;

        children.push_back(std::move(newChild));
        return rawPtr;
    }

    template <typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        // Asegurarnos de que T hereda de Component
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

        auto newComponent = std::make_unique<T>(std::forward<Args>(args)...);
        T* rawPtr = newComponent.get();

        // Asignamos la referencia al GameObject padre
        newComponent->owner = this;

        components.push_back(std::move(newComponent));
        return rawPtr;
    }

    template <typename T>
    T* GetComponent() {
        for (const auto& comp : components) {
            if (auto ptr = dynamic_cast<T*>(comp.get())) {
                return ptr;
            }
        }
        return nullptr;
    }

    GameObject* GetParent() const { return parent; }

    const std::vector<std::unique_ptr<GameObject>>& GetChildren() const { return children; }
};
