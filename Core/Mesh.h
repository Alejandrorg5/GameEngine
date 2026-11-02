#pragma once

#include <vector>
#include <glad/glad.h>
#include "../Modules/Render.h"

class Mesh {
public:
    unsigned int VAO, VBO, EBO;
    std::vector<Render::Vertex> vertices;
    std::vector<unsigned int> indices;
    GLuint textureID = 0;

    Mesh() {};

    void SetupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Render::Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Atributo de Posición
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Render::Vertex), (void*)0);

        // Atributo de Normales
        glEnableClientState(GL_NORMAL_ARRAY); // O usar glVertexAttribPointer si usas shaders programables en el futuro
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Render::Vertex), (void*)offsetof(Render::Vertex, Normal));

        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, sizeof(Render::Vertex), (void*)offsetof(Render::Vertex, TexCoords));

        // Atributo de Coordenadas de Textura
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Render::Vertex), (void*)offsetof(Render::Vertex, TexCoords));

        glBindVertexArray(0);
    }

    void Draw() {
        if (textureID != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Desvincular textura por buena práctica
        if (textureID != 0) {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
};