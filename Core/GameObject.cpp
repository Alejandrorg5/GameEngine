#include "GameObject.h"


GameObject::GameObject()
{

}

// Destructor
GameObject::~GameObject()
{
}

void GameObject::Update(float deltaTime) {
    for (const auto& comp : components) {
        comp->Update(deltaTime);
    }
}