#pragma once
#include <glad/glad.h>
#include <IL/il.h>
#include <string>

class TextureLoader {
public:
    static GLuint LoadTexture(const std::string& path);

    static GLuint LoadMemory(const void* data, ILuint size, const std::string& label);

    static GLuint Upload(int width, int height, GLenum format, const void* pixels);
};
