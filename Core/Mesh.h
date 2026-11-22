#pragma once

#include <vector>
#include <glad/glad.h>
#include "../Modules/Render.h"

class Mesh {
public:
    std::vector<Render::Vertex> vertices;
    std::vector<unsigned int> indices;

    Mesh() = default;
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;
    ~Mesh();
    void SetupMesh();
    void Draw() const;

private:
    void CleanUp();
    GLuint VAO = 0, VBO = 0, EBO = 0;
};
