#pragma once
#include "Component.h"
#include <glad/glad.h>
#include <string>

class TextureComponent final : public Component {
public:
    ~TextureComponent() override;
    void SetTexture(GLuint id, const std::string& path = {});
    GLuint Id() const { return textureID; }
    const std::string& Path() const { return filePath; }
    int Width() const { return width; }
    int Height() const { return height; }
    bool ShowsChecker() const { return showChecker; }
    void SetChecker(bool enabled) { showChecker = enabled; }

private:
    GLuint textureID = 0;
    std::string filePath;
    int width = 0, height = 0;
    bool showChecker = false;
};
