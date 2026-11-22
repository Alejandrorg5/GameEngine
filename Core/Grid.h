#pragma once
#include <glad/glad.h>

class Grid {
private:
    GLuint VAO = 0, VBO = 0;
    int vertexCount = 0;

public:
    Grid() = default;
    Grid(const Grid&) = delete;
    Grid& operator=(const Grid&) = delete;
    ~Grid();
    void Init(int size = 10, float step = 1.0f);
    void Draw();
    void CleanUp();
};
