#pragma once
#include <glad/glad.h>
#include <vector>

class Grid {
private:
    GLuint VAO = 0, VBO = 0;
    int vertexCount = 0;

public:
    void Init(int size = 10, float step = 1.0f) {
        std::vector<float> vertices;

        // Generar líneas paralelas a los ejes
        for (int i = -size; i <= size; ++i) {
            float pos = i * step;

            // Línea paralela al eje Z (variando X)
            // Posición (X, Y, Z) + Color (R, G, B)
            vertices.push_back(pos); vertices.push_back(0.0f); vertices.push_back((float)-size * step);
            vertices.push_back(0.5f); vertices.push_back(0.5f); vertices.push_back(0.5f); // Color gris

            vertices.push_back(pos); vertices.push_back(0.0f); vertices.push_back((float)size * step);
            vertices.push_back(0.5f); vertices.push_back(0.5f); vertices.push_back(0.5f);

            // Línea paralela al eje X (variando Z)
            vertices.push_back((float)-size * step); vertices.push_back(0.0f); vertices.push_back(pos);
            vertices.push_back(0.5f); vertices.push_back(0.5f); vertices.push_back(0.5f);

            vertices.push_back((float)size * step); vertices.push_back(0.0f); vertices.push_back(pos);
            vertices.push_back(0.5f); vertices.push_back(0.5f); vertices.push_back(0.5f);
        }

        vertexCount = static_cast<int>(vertices.size() / 6); // 3 para posición + 3 para color

        // Configurar VAO y VBO en OpenGL
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // Atributo de posición (Location 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Atributo de color (Location 3)
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(3);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void Draw() {
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, vertexCount);
        glBindVertexArray(0);
    }

    ~Grid() { CleanUp(); }

    void CleanUp() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        VAO = VBO = 0;
        vertexCount = 0;
    }
};