#include "GameObject.h"


GameObject::GameObject()
{
    AddComponent<TransformComponent>();
    AddComponent<MeshComponent>();
    AddComponent<TextureComponent>();
}

GameObject::GameObject(string n)
{
    name = n;
    AddComponent<TransformComponent>();
    AddComponent<MeshComponent>();
    AddComponent<TextureComponent>();
}

// Destructor
GameObject::~GameObject()
{
}

void GameObject::Update(float deltaTime) {

    for (const auto& comp : components) {
        comp->Update(deltaTime);
    }

    for (const auto& child : children) {
        child->Update(deltaTime);
    }
}