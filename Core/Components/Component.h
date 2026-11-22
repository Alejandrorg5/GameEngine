#pragma once

class GameObject;

class Component {
public:
    virtual ~Component() = default;
    virtual void Update(float) {}

protected:
    GameObject* owner = nullptr;

private:
    friend class GameObject;
};
