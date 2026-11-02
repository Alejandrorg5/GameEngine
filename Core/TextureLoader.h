#pragma once

#include <glad/glad.h>
#include <IL/il.h>
#include <string>
#include <iostream>

class TextureLoader {
public:
    static GLuint LoadTexture(const std::string& path) {
        // 1. Inicializar DevIL (asegúrate de llamar a ilInit() una sola vez en el Awake/Start de tu motor)
        // ilInit(); 

        ILuint imageID;
        ilGenImages(1, &imageID);
        ilBindImage(imageID);

        // 2. Cargar la imagen desde la ruta
        if (!ilLoadImage(path.c_str())) {
            std::cout << "ERROR::DEVIL:: No se pudo cargar la imagen: " << path << std::endl;
            ilDeleteImages(1, &imageID);
            return 0;
        }
        ilEnable(IL_ORIGIN_SET);
        ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

        // 3. Asegurar que los datos estén en un formato estándar (RGBA o RGB) de 8 bits por canal
        ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

        GLuint textureID = 0;
        int width = ilGetInteger(IL_IMAGE_WIDTH);
        int height = ilGetInteger(IL_IMAGE_HEIGHT);
        unsigned int format = GL_RGBA; // Ya que convertimos a IL_RGBA

        // 4. Crear la textura en OpenGL
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Parámetros de la textura
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Enviar los píxeles de DevIL a OpenGL
       
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, ilGetData());

        // Generar Mipmaps opcionalmente para mayor calidad
        glGenerateMipmap(GL_TEXTURE_2D);

        // 5. Limpiar la imagen de la memoria temporal de DevIL
        ilDeleteImages(1, &imageID);
        glBindTexture(GL_TEXTURE_2D, 0);

        return textureID;
    }
};