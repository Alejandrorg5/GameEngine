#pragma once
#include <glad/glad.h>
#include <IL/il.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <limits>
#include "../Modules/Log.h"

class TextureLoader {
public:
    static GLuint LoadTexture(const std::string& path) {
        std::ifstream file(std::filesystem::u8path(path), std::ios::binary | std::ios::ate);
        if (!file) { LOG("DevIL: cannot open %s", path.c_str()); return 0; }
        const auto size = file.tellg();
        if (size <= 0 || static_cast<unsigned long long>(size) > std::numeric_limits<ILuint>::max()) {
            LOG("DevIL: invalid image size: %s", path.c_str()); return 0;
        }
        std::vector<char> bytes(static_cast<size_t>(size));
        file.seekg(0);
        if (!file.read(bytes.data(), static_cast<std::streamsize>(bytes.size()))) return 0;
        return LoadMemory(bytes.data(), static_cast<ILuint>(bytes.size()), path);
    }

    static GLuint LoadMemory(const void* data, ILuint size, const std::string& label) {
        ILuint image = 0;
        ilGenImages(1, &image);
        ilBindImage(image);
        // Assimp flips UVs; keep all decoded images in the same top-left convention.
        ilEnable(IL_ORIGIN_SET);
        ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
        if (!ilLoadL(IL_TYPE_UNKNOWN, data, size) || !ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE)) {
            LOG("DevIL: image decode failed (%u): %s", ilGetError(), label.c_str());
            ilDeleteImages(1, &image);
            return 0;
        }
        const int width = ilGetInteger(IL_IMAGE_WIDTH), height = ilGetInteger(IL_IMAGE_HEIGHT);
        const GLuint texture = Upload(width, height, GL_RGBA, ilGetData());
        ilDeleteImages(1, &image);
        LOG("DevIL: loaded %s (%d x %d)", label.c_str(), width, height);
        return texture;
    }

    static GLuint Upload(int width, int height, GLenum format, const void* pixels) {
        if (width <= 0 || height <= 0 || !pixels) return 0;
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        return texture;
    }
};
