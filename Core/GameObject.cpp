#define GLM_ENABLE_EXPERIMENTAL
#include "GameObject.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <cmath>

glm::mat4 GameObject::GetGlobalMatrix() {
    const auto* transform = GetComponent<TransformComponent>();
    const glm::mat4 local = transform ? transform->GetModelMatrix() : glm::mat4(1.0f);
    return parent ? parent->GetGlobalMatrix() * local : local;
}


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


bool GameObject::GetWorldBounds(glm::vec3& low, glm::vec3& high) {
    bool found = false;
    const auto include = [&](const glm::vec3& a, const glm::vec3& b) {
        if (!found) { low = a; high = b; found = true; }
        else { low = glm::min(low, a); high = glm::max(high, b); }
    };
    if (auto* mesh = GetComponent<MeshComponent>()) {
        const auto matrix = GetGlobalMatrix();
        for (const auto& item : mesh->meshes) for (const auto& vertex : item.vertices) {
            const glm::vec3 point(matrix * glm::vec4(vertex.Position, 1.0f));
            include(point, point);
        }
    }
    for (const auto& child : children) {
        glm::vec3 a, b;
        if (child->GetWorldBounds(a, b)) include(a, b);
    }
    return found;
}


bool GameObject::SetGlobalMatrix(const glm::mat4& matrix) {
    auto* transform = GetComponent<TransformComponent>();
    if (!transform) return false;
    glm::mat4 local = matrix;
    if (parent) {
        const auto parentMatrix = parent->GetGlobalMatrix();
        if (std::abs(glm::determinant(parentMatrix)) < 1e-12f) return false;
        local = glm::inverse(parentMatrix) * matrix;
    }
    glm::vec3 position, scale, skew;
    glm::quat rotation;
    glm::vec4 perspective;
    if (!glm::decompose(local, scale, rotation, position, skew, perspective)) return false;
    const glm::vec3 angles = glm::degrees(glm::eulerAngles(glm::normalize(rotation)));
    TransformComponent candidate;
    candidate.position = position; candidate.rotation = angles; candidate.scale = scale;
    const auto recomposed = candidate.GetModelMatrix();
    float magnitude = 1.0f, error = 0.0f;
    for (int column = 0; column < 4; ++column) for (int row = 0; row < 4; ++row) {
        if (!std::isfinite(recomposed[column][row]) || !std::isfinite(local[column][row])) return false;
        magnitude = std::max(magnitude, std::abs(local[column][row]));
        error = std::max(error, std::abs(local[column][row] - recomposed[column][row]));
    }
    // TRS cannot represent shear from rotating below a non-uniformly scaled parent.
    if (error > 1e-4f * magnitude) return false;
    transform->position = position; transform->rotation = angles; transform->scale = scale;
    return true;
}
