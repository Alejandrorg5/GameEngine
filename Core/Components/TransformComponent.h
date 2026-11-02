#pragma once

#include "Component.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class TransformComponent : public Component {

public:

	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);;
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);;
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);;

	void Enable() { active = true; };
	void Disable() { active = false; };
    void Update(float dt) override {};

    glm::mat4 GetModelMatrix() const {
        glm::mat4 transform(1.0f);

        // 1. Traslación
        transform = glm::translate(transform, position);

        // 2. Rotación (Euler angles en radianes)
        transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        // 3. Escala
        transform = glm::scale(transform, scale);

        return transform;
    }
};