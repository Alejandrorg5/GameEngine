#pragma once
#include <glm/glm.hpp>

class GameObject;

class Camera {
private:
    glm::vec3 target{ 0.0f };
    float distance = 15.0f;
    float yaw = -90.0f;
    float pitch = 20.0f;
    float sensitivity = 0.2f;
    float zoomSensitivity = 2.0f;
    float moveSpeed = 8.0f;
    glm::vec3 position{ 0.0f };

public:
    
    glm::vec3 Offset() const;

    glm::mat4 Projection(float aspect) const;

    glm::mat4 GetViewMatrix();

    void Focus(GameObject* object, bool frame);

    void Update(float dt, GameObject* selected, bool mouseAllowed = true, bool keyboardAllowed = true);

    /* Getters y Setters */

    float* GetDistance() { return &distance; }
    float* GetYaw() { return &yaw; }
    float* GetPitch() { return &pitch; }
    float* GetSensitivity() { return &sensitivity; }
    float* GetZoomSens() { return &zoomSensitivity; }
    float* GetMoveSpeed() { return &moveSpeed; }

    void SetDistance(float val) { distance = val; }
    void SetYaw(float val) {  yaw = val; }
    void SetPitch(float val) { pitch = val; }
    void SetSensitivity(float val) { sensitivity = val; }
    void SetZoomSens(float val) { zoomSensitivity = val; }
    void SetMoveSpeed(float val) { moveSpeed = val; }
};
