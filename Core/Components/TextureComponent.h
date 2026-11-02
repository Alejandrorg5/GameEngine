#pragma once

#include "Component.h"
#include <glad/glad.h>
#include <string>

class TextureComponent : public Component {
public:
    GLuint textureID = 0;
    std::string filePath = "";
    int width = 0;
    int height = 0;
    bool showChecker = false;

    ~TextureComponent() override {
        // Opcional: limpiar la textura de OpenGL si es necesario al destruir el componente
        if (textureID != 0) {
            glDeleteTextures(1, &textureID);
        }
    }

    void Update(float dt) override {
        // Las texturas normalmente no necesitan lógica por fotograma, pero el método es obligatorio por la interfaz
    }

    void SetTexture(GLuint id, const std::string& path = "") {
        if (textureID != 0 && textureID != id) glDeleteTextures(1, &textureID);
        textureID = id;
        filePath = path;
        width = height = 0;
        if (textureID) {
            GLint previous = 0;
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
            glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previous));
        }
    }
};
