#pragma once
#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <array>
#include "../Core/Camera.h"
#include "Module.h"

class Scene;
class Editor final : public Module {
public:
    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;
private:
    void Draw(Scene& scene, float dt);
    static void ProcessEvent(const SDL_Event& event);
    std::string layoutPath;
    Camera camera;
    void Hierarchy(GameObject& object, Scene& scene);
    void Inspector(GameObject* selectedObject);
    void Configuration(Scene& scene);
    void About();
    bool ResizeTarget(int width, int height);
    void DeleteDialog(Scene& scene);
    int gizmoOperation = 0;
    bool gizmoLocal = true;
    bool transformRejected = false;
    bool revealSelection = false;
    GameObject* deleteRequest = nullptr;
    GameObject* deleteTarget = nullptr;
    GLuint framebuffer = 0, colorTexture = 0, depthBuffer = 0;
    int targetWidth = 0, targetHeight = 0;
    bool showHierarchy = true, showInspector = true, showScene = true, showConsole = true;
    bool showConfiguration = false, showAbout = false;
    std::array<float, 120> fps{};
    int fpsOffset = 0;
};
