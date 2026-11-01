#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>

#include "Components/Component.h"
#include "Components/TransformComponent.h"
#include "Components/MeshComponent.h"
#include "Components/MaterialComponent.h"

using namespace std;

class GameObject;


class GameObject {

public:
    GameObject();

    // Destructor
    virtual ~GameObject();

    bool active;
    string name;

    vector<unique_ptr<Component>> components;

    void Update(float dt);

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

};