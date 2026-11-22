#pragma once
#include <glm/glm.hpp>

class GameObject;

class Camera {
public:
    glm::vec3 target{0.0f};
    float distance = 15.0f;
    float yaw = -90.0f;
    float pitch = 20.0f;
    float sensitivity = 0.2f;
    float zoomSensitivity = 2.0f;
    float moveSpeed = 8.0f;
    glm::vec3 position{0.0f};

    glm::vec3 Offset() const;

    glm::mat4 Projection(float aspect) const;

    glm::mat4 GetViewMatrix();

    void Focus(GameObject* object, bool frame);

    void Update(float dt, GameObject* selected, bool mouseAllowed = true, bool keyboardAllowed = true);
};
