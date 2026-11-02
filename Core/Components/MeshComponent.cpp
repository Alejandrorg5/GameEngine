#include "MeshComponent.h"
#include "../GameObject.h"
#include "../../Modules/Engine.h"
#include <cmath>

void MeshComponent::Render() {
    if (!owner || meshes.empty()) return;
    const auto* texture = owner->GetComponent<TextureComponent>();
    auto& renderer = *Engine::GetInstance().render;
    const glm::mat4 model = owner->GetGlobalMatrix();
    const GLuint textureID = texture ? (texture->showChecker ? renderer.GetCheckerTexture() : texture->textureID) : 0;
    renderer.SetObject(model, textureID);
    for (const auto& mesh : meshes) mesh.Draw();

    if (!showVertexNormals && !showFaceNormals) return;
    const glm::mat3 basis(model);
    const bool invertible = std::abs(glm::determinant(basis)) > 1e-8f;
    const glm::mat3 normalMatrix = invertible ? glm::transpose(glm::inverse(basis)) : glm::mat3(1.0f);
    std::vector<glm::vec3> vertexLines, faceLines;
    const auto addLine = [this](std::vector<glm::vec3>& lines, const glm::vec3& position, const glm::vec3& normal) {
        const float length = glm::length(normal);
        if (length < 1e-8f) return;
        lines.push_back(position);
        lines.push_back(position + normal / length * normalLength);
    };
    for (const auto& mesh : meshes) {
        if (showVertexNormals && invertible) {
            for (const auto& vertex : mesh.vertices)
                addLine(vertexLines, glm::vec3(model * glm::vec4(vertex.Position, 1.0f)), normalMatrix * vertex.Normal);
        }
        if (showFaceNormals) {
            for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
                const glm::vec3 a(model * glm::vec4(mesh.vertices[mesh.indices[i]].Position, 1.0f));
                const glm::vec3 b(model * glm::vec4(mesh.vertices[mesh.indices[i + 1]].Position, 1.0f));
                const glm::vec3 c(model * glm::vec4(mesh.vertices[mesh.indices[i + 2]].Position, 1.0f));
                addLine(faceLines, (a + b + c) / 3.0f, glm::cross(b - a, c - a));
            }
        }
    }
    renderer.DrawLines(vertexLines, glm::vec3(0.0f, 1.0f, 1.0f));
    renderer.DrawLines(faceLines, glm::vec3(1.0f, 1.0f, 0.0f));
}

void MeshComponent::Update(float deltaTime) {
    Render();
}
