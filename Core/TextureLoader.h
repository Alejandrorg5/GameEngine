#pragma once
#include <glad/glad.h>
#include <IL/il.h>
#include <assimp/scene.h>
#include <iostream>

class TextureLoader {
public:
    // Función para cargar texturas desde archivos externos (como tenías antes)
    static GLuint LoadTexture(const std::string& path) {
        ILuint imageID;
        ilGenImages(1, &imageID);
        ilBindImage(imageID);

        if (!ilLoadImage(path.c_str())) {
            ilDeleteImages(1, &imageID);
            return 0;
        }

        ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
        return CreateOpenGLTexture(ilGetData(), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), imageID);
    }

    // NUEVA: Función para cargar texturas incrustadas (embebidas en la memoria de Assimp)
    static GLuint LoadEmbeddedTexture(const aiTexture* embeddedTexture) {
        if (!embeddedTexture) return 0;

        ILuint imageID;
        ilGenImages(1, &imageID);
        ilBindImage(imageID);

        // Si mHeight es 0, significa que los datos están comprimidos (ej: formato PNG o JPG en crudo)
        bool success = false;
        if (embeddedTexture->mHeight == 0) {
            // ilLoadL lee directamente el buffer de memoria comprimido
            success = ilLoadL(IL_TYPE_UNKNOWN, (const void*)embeddedTexture->pcData, embeddedTexture->mWidth);
            // Nota: Dependiendo de la versión de DevIL, si el formato comprimido da problemas, 
            // a veces se usa ilLoadL(IL_JPG) o IL_PNG según el hint (mAchHint).
        }
        else {
            // Si la imagen no está comprimida (píxeles directos)
            // ilLoadL(IL_RGBA, ...) o similar usando mWidth y mHeight
            // (El 90% de los FBX embebidos usan mHeight == 0 con PNG/JPG comprimido dentro).
        }

        if (!success) {
            std::cout << "ERROR::DEVIL:: No se pudo cargar la textura embebida." << std::endl;
            ilDeleteImages(1, &imageID);
            return 0;
        }

        ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
        return CreateOpenGLTexture(ilGetData(), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), imageID);
    }

private:
    static GLuint CreateOpenGLTexture(const ILubyte* data, int width, int height, ILuint imageID) {
        GLuint textureID = 0;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        ilDeleteImages(1, &imageID);
        glBindTexture(GL_TEXTURE_2D, 0);

        return textureID;
    }
};