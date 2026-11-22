#include "Mesh.h"
#include <cstddef>
#include <utility>

Mesh::Mesh(Mesh&& other) noexcept { *this = std::move(other); }
Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        CleanUp();
        VAO = std::exchange(other.VAO, 0);
        VBO = std::exchange(other.VBO, 0);
        EBO = std::exchange(other.EBO, 0);
        vertices = std::move(other.vertices);
        indices = std::move(other.indices);
    }
    return *this;
}
Mesh::~Mesh() { CleanUp(); }

void Mesh::CleanUp() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    VAO = VBO = EBO = 0;
}

void Mesh::SetupMesh() {
    CleanUp();
    if (vertices.empty() || indices.empty()) return;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Render::Vertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Render::Vertex),
        reinterpret_cast<void*>(offsetof(Render::Vertex, Position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Render::Vertex),
        reinterpret_cast<void*>(offsetof(Render::Vertex, Normal)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Render::Vertex),
        reinterpret_cast<void*>(offsetof(Render::Vertex, TexCoords)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::Draw() const {
    if (!VAO || indices.empty()) return;
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
