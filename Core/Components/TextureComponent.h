#pragma once

#include "Component.h"
#include <glad/glad.h>
#include <string>

class TextureComponent : public Component {
public:
    GLuint textureID = 0;
    std::string filePath = "";

    ~TextureComponent() {
        // Opcional: limpiar la textura de OpenGL si es necesario al destruir el componente
        if (textureID != 0) {
            glDeleteTextures(1, &textureID);
        }
    }

    void Update(float dt) override {
        // Las texturas normalmente no necesitan lógica por fotograma, pero el método es obligatorio por la interfaz
    }

    void SetTexture(GLuint id, const std::string& path = "") {
        textureID = id;
        filePath = path;
    }
};