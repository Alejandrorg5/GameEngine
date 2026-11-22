#include "TextureComponent.h"

TextureComponent::~TextureComponent() {
    if (textureID) glDeleteTextures(1, &textureID);
}

void TextureComponent::SetTexture(GLuint id, const std::string& path) {
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
