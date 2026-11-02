#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../Modules/Engine.h"
#include "../Modules/Input.h"

class Camera {
public:
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    float distance = 15.0f;
    float yaw = -90.0f;
    float pitch = 20.0f;
    
    float sensitivity = 0.2f;
    float zoomSensitivity = 2.0f;
    float panSensitivity = 0.02f;

    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::mat4 GetViewMatrix() {
        float radYaw = glm::radians(yaw);
        float radPitch = glm::radians(pitch);

        position.x = target.x + distance * cos(radPitch) * cos(radYaw);
        position.y = target.y + distance * sin(radPitch);
        position.z = target.z + distance * cos(radPitch) * sin(radYaw);

        return glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void Update() {
       
        // 1. ZOOM con la rueda del ratón
        int wheel = Engine::GetInstance().input->GetMouseWheel();
        if (wheel != 0) {
            distance -= wheel * zoomSensitivity;
            if (distance < 1.0f) distance = 1.0f;
            if (distance > 500.0f) distance = 500.0f;
        }

        // ORBITAR (Botón izquierdo del raton + L alt)
        if (Engine::GetInstance().input->GetMouseButtonDown(1) == KEY_DOWN || Engine::GetInstance().input->GetMouseButtonDown(1) == KEY_REPEAT) {
            if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_LALT) == KEY_DOWN || Engine::GetInstance().input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT) {
                int motionX = 0, motionY = 0;
                Engine::GetInstance().input->GetMouseMotion(motionX, motionY);

                // Solo actualizamos si realmente ha habido movimiento del ratón en este frame
                if (motionX != 0 || motionY != 0) {
                    yaw += (float)motionX * sensitivity;
                    pitch += (float)motionY * sensitivity; // Invertido natural del eje Y

                    // Limitar el pitch para evitar que la cámara dé la vuelta completa verticalmente
                    if (pitch > 89.0f)  pitch = 89.0f;
                    if (pitch < -89.0f) pitch = -89.0f;
                }
            }
        }

        // 3. PANORÁMICA (Botón derecho del ratón)
        if (Engine::GetInstance().input->GetMouseButtonDown(3) == KEY_DOWN || Engine::GetInstance().input->GetMouseButtonDown(3) == KEY_REPEAT) {
            int motionX = 0, motionY = 0;
            Engine::GetInstance().input->GetMouseMotion(motionX, motionY);

            if (motionX != 0 || motionY != 0) {
                float radYaw = glm::radians(yaw);
                glm::vec3 forward = glm::normalize(target - position);
                glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
                glm::vec3 up = glm::cross(right, forward);

                target -= right * (float)motionX * panSensitivity * (distance / 10.0f);
                target += up * (float)motionY * panSensitivity * (distance / 10.0f);
            }
        }
    }
};