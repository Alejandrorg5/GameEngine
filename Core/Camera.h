#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <limits>
#include <functional>
#include "../Modules/Engine.h"
#include "../Modules/Input.h"
#include "GameObject.h"

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

    glm::vec3 Offset() const {
        const float y = glm::radians(yaw), p = glm::radians(pitch);
        return distance * glm::vec3(cos(p) * cos(y), sin(p), cos(p) * sin(y));
    }

    glm::mat4 GetViewMatrix() {
        position = target + Offset();
        return glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void Focus(GameObject* object, bool frame) {
        if (!object) return;
        glm::vec3 low, high;
        if (!object->GetWorldBounds(low, high)) return;
        position = target + Offset();
        target = (low + high) * 0.5f;
        if (frame) {
            distance = std::max(1.0f, glm::length(high - low) * 0.5f / sin(glm::radians(22.5f)) * 1.1f);
        } else {
            const glm::vec3 delta = position - target;
            distance = std::max(glm::length(delta), 0.1f);
            yaw = glm::degrees(atan2(delta.z, delta.x));
            pitch = glm::degrees(asin(std::clamp(delta.y / distance, -1.0f, 1.0f)));
        }
    }

    void Update(float dt, GameObject* selected, bool mouseAllowed = true, bool keyboardAllowed = true) {
        auto& input = *Engine::GetInstance().input;
        const auto held = [&](SDL_Scancode key) {
            const auto state = input.GetKey(key);
            return state == KEY_DOWN || state == KEY_REPEAT;
        };
        const auto mouseHeld = [&](int button) {
            const auto state = input.GetMouseButtonDown(button);
            return state == KEY_DOWN || state == KEY_REPEAT;
        };
        if (keyboardAllowed && input.GetKey(SDL_SCANCODE_F) == KEY_DOWN) Focus(selected, true);
        if (!mouseAllowed) return;
        position = target + Offset();
        int dx = 0, dy = 0;
        input.GetMouseMotion(dx, dy);
        if ((held(SDL_SCANCODE_LALT) || held(SDL_SCANCODE_RALT)) && mouseHeld(1)) {
            if (input.GetMouseButtonDown(1) == KEY_DOWN) Focus(selected, false);
            yaw += dx * sensitivity;
            pitch = std::clamp(pitch + dy * sensitivity, -89.0f, 89.0f);
        } else if (mouseHeld(3)) {
            yaw += dx * sensitivity;
            pitch = std::clamp(pitch + dy * sensitivity, -89.0f, 89.0f);
            target = position - Offset();
            if (keyboardAllowed) {
                const glm::vec3 forward = glm::normalize(target - position);
                const glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
                glm::vec3 motion(0.0f);
                if (held(SDL_SCANCODE_W)) motion += forward;
                if (held(SDL_SCANCODE_S)) motion -= forward;
                if (held(SDL_SCANCODE_D)) motion += right;
                if (held(SDL_SCANCODE_A)) motion -= right;
                const float speed = moveSpeed * ((held(SDL_SCANCODE_LSHIFT) || held(SDL_SCANCODE_RSHIFT)) ? 2.0f : 1.0f);
                if (glm::length(motion) > 0.0f) target += glm::normalize(motion) * speed * dt;
            }
        }
        distance = std::max(distance - input.GetMouseWheel() * zoomSensitivity, 0.1f);
    }
};
